// metadata.cpp
// Implementation of file-metadata inspection.
//
// Important: we use ::lstat() — NOT ::stat() — as the primary call so
// that a symlink itself is reported, not its target.  We additionally
// call ::stat() only when the path is a symlink, to also report the
// target's properties.  This avoids infinite loops in symlink cycles.

#include "metadata.h"
#include "utils.h"
#include "activity_log.h"

#include <iostream>
#include <iomanip>
#include <filesystem>
#include <cstring>          // strerror
#include <ctime>
#include <unistd.h>         // access()
#include <sys/stat.h>       // lstat, stat, mode_t
#include <pwd.h>            // getpwuid
#include <grp.h>            // getgrgid

namespace fs = std::filesystem;

static ActivityLog& logger = ActivityLog::getInstance();

// Convert st_mode type bits to a human-readable type label.
static std::string fileTypeLabel(mode_t m) {
    if (S_ISREG(m))  return "Regular file";
    if (S_ISDIR(m))  return "Directory";
    if (S_ISLNK(m))  return "Symbolic link";
    if (S_ISCHR(m))  return "Character device";
    if (S_ISBLK(m))  return "Block device";
    if (S_ISFIFO(m)) return "FIFO (named pipe)";
    if (S_ISSOCK(m)) return "Socket";
    return "Unknown";
}

// Format st_mtime as YYYY-MM-DD HH:MM:SS
static std::string formatTime(const time_t t) {
    std::tm* lt = std::localtime(&t);
    char buf[32] = {0};
    if (lt) std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
    return std::string(buf);
}

// Look up a user name from uid; falls back to the numeric id.
static std::string uidToName(uid_t uid) {
    struct passwd* pw = ::getpwuid(uid);
    if (pw) return std::string(pw->pw_name) + " (" + std::to_string(uid) + ")";
    return std::to_string(uid);
}
static std::string gidToName(gid_t gid) {
    struct group* gr = ::getgrgid(gid);
    if (gr) return std::string(gr->gr_name) + " (" + std::to_string(gid) + ")";
    return std::to_string(gid);
}

bool Metadata::showMetadata(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    struct stat st;
    // Use lstat to inspect the symlink itself.
    if (::lstat(path.c_str(), &st) != 0) {
        utils::printError("lstat() failed: " + std::string(strerror(errno)));
        logger.log("METADATA", path, "", "FAILURE", strerror(errno));
        return false;
    }
    bool isLink = S_ISLNK(st.st_mode);

    // Use std::filesystem to compute absolute path & extension safely.
    std::error_code ec;
    fs::path abs = fs::weakly_canonical(fs::absolute(path, ec));
    std::string absStr = abs.string();
    std::string name = abs.filename().string();
    std::string ext  = utils::getExtension(path);

    utils::printHeader("FILE METADATA");
    std::cout << std::left;
    std::cout << "  Name             : " << name << "\n";
    std::cout << "  Absolute path    : " << absStr << "\n";
    std::cout << "  Extension        : " << (ext.empty() ? "(none)" : "." + ext) << "\n";
    std::cout << "  File type        : " << fileTypeLabel(st.st_mode) << "\n";
    std::cout << "  Size (bytes)     : " << st.st_size << "\n";
    std::cout << "  Size (human)     : " << utils::formatSize(st.st_size) << "\n";
    std::cout << "  Last modified    : " << formatTime(st.st_mtime) << "\n";
    std::cout << "  Last accessed    : " << formatTime(st.st_atime) << "\n";
    std::cout << "  Last status chg  : " << formatTime(st.st_ctime) << "\n";
    std::cout << "  Inode            : " << st.st_ino << "\n";
    std::cout << "  Hard links       : " << st.st_nlink << "\n";
    std::cout << "  Device ID        : " << st.st_dev << "\n";
    std::cout << "  Owner (uid)      : " << uidToName(st.st_uid) << "\n";
    std::cout << "  Group (gid)      : " << gidToName(st.st_gid) << "\n";
    std::cout << "  Permissions sym  : " << utils::permissionToString(st.st_mode) << "\n";
    std::cout << "  Permissions oct  : " << utils::permissionToOctal(st.st_mode) << "\n";

    // Current-user access checks via access()
    bool r = (::access(path.c_str(), R_OK) == 0);
    bool w = (::access(path.c_str(), W_OK) == 0);
    bool x = (::access(path.c_str(), X_OK) == 0);
    bool e = (::access(path.c_str(), F_OK) == 0);
    std::cout << "  Exists (F_OK)    : " << (e ? "yes" : "no") << "\n";
    std::cout << "  Readable         : " << (r ? "yes" : "no") << "\n";
    std::cout << "  Writable         : " << (w ? "yes" : "no") << "\n";
    std::cout << "  Executable       : " << (x ? "yes" : "no") << "\n";
    std::cout << "  Hidden file      : " << (utils::isHidden(name) ? "yes" : "no") << "\n";
    std::cout << "  Symbolic link    : " << (isLink ? "yes" : "no") << "\n";

    if (isLink) {
        // Also try to read the target via stat()
        struct stat tgt;
        if (::stat(path.c_str(), &tgt) == 0) {
            std::cout << "\n  --- Target of symlink ---\n";
            std::cout << "  Target type      : " << fileTypeLabel(tgt.st_mode) << "\n";
            std::cout << "  Target size      : " << tgt.st_size
                      << "  (" << utils::formatSize(tgt.st_size) << ")\n";
            std::cout << "  Target perms     : "
                      << utils::permissionToString(tgt.st_mode) << " ("
                      << utils::permissionToOctal(tgt.st_mode) << ")\n";
        } else {
            std::cout << "\n  Target of symlink: <broken link or inaccessible>\n";
        }
        // Show the textual target of the link via readlink().
        std::string buf(4096, '\0');
        ssize_t n = ::readlink(path.c_str(), &buf[0], buf.size());
        if (n > 0) {
            buf.resize(static_cast<size_t>(n));
            std::cout << "  Link points to   : " << buf << "\n";
        }
    }

    std::cout << "\n  POSIX calls used: lstat(), stat() [if symlink], "
                 "access(), getpwuid(), getgrgid(), readlink().\n";
    logger.log("METADATA", path, "", "SUCCESS", "");
    return true;
}

void Metadata::showMenu() {
    while (true) {
        utils::printHeader("FILE METADATA");
        std::cout << "  1. Show metadata for a file or directory\n"
                  << "  0. Return to Main Menu\n\n";
        int choice = -1;
        if (!utils::getInt("Enter your choice", choice)) {
            if (utils::eofReached()) return;          // piped input ran out
            utils::printError("Invalid input.");
            utils::pauseScreen();
            continue;
        }
        if (choice == 0) return;
        if (choice == 1) {
            std::string p = utils::getInput("Enter path");
            showMetadata(p);
        } else {
            utils::printError("Invalid choice.");
        }
        utils::pauseScreen();
    }
}
