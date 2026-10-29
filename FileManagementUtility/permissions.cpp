// permissions.cpp
// Implementation of the Linux permissions module.

#include "permissions.h"
#include "utils.h"
#include "activity_log.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <unistd.h>      // access, getuid, getgid
#include <sys/stat.h>    // chmod, stat

namespace fs = std::filesystem;

static ActivityLog& logger = ActivityLog::getInstance();

// ---------------------------------------------------------------------
// Parse "ddd" octal -> mode_t (only user/group/other bits)
// ---------------------------------------------------------------------
bool Permissions::parseOctal(const std::string& s, mode_t& out) {
    if (s.size() != 3) return false;
    for (char c : s) if (c < '0' || c > '7') return false;
    int o = s[0] - '0';
    int g = s[1] - '0';
    int w = s[2] - '0';
    out = static_cast<mode_t>((o << 6) | (g << 3) | w);
    return true;
}

// ---------------------------------------------------------------------
// View permissions
// ---------------------------------------------------------------------
bool Permissions::viewPermissions(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    struct stat st;
    if (::lstat(path.c_str(), &st) != 0) {
        utils::printError("lstat() failed: " + std::string(strerror(errno)));
        return false;
    }
    std::string sym = utils::permissionToString(st.st_mode);
    std::string oct = utils::permissionToOctal(st.st_mode);

    utils::printSubHeader("Permissions of " + path);
    std::cout << "  Symbolic : " << sym << "\n"
              << "  Octal    : " << oct << "\n\n";
    std::cout << "  Owner (rwx) : " << sym.substr(0, 3) << "  "
              << "Read="  << (sym[0]=='r'?'Y':'N') << "  "
              << "Write=" << (sym[1]=='w'?'Y':'N') << "  "
              << "Exec="  << (sym[2]=='x'?'Y':'N') << "\n";
    std::cout << "  Group (rwx) : " << sym.substr(3, 3) << "  "
              << "Read="  << (sym[3]=='r'?'Y':'N') << "  "
              << "Write=" << (sym[4]=='w'?'Y':'N') << "  "
              << "Exec="  << (sym[5]=='x'?'Y':'N') << "\n";
    std::cout << "  Other (rwx) : " << sym.substr(6, 3) << "  "
              << "Read="  << (sym[6]=='r'?'Y':'N') << "  "
              << "Write=" << (sym[7]=='w'?'Y':'N') << "  "
              << "Exec="  << (sym[8]=='x'?'Y':'N') << "\n";
    std::cout << "\n  Current process uid: " << ::getuid()
              << "  gid: " << ::getgid() << "\n";
    logger.log("VIEW_PERMS", path, "", "SUCCESS", sym + " " + oct);
    return true;
}

// ---------------------------------------------------------------------
// Set permissions using octal mode
// ---------------------------------------------------------------------
bool Permissions::setOctalMode(const std::string& path,
                                const std::string& octalStr) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::exists(path)) {
        utils::printError("Path does not exist.");
        return false;
    }
    mode_t newMode;
    if (!parseOctal(octalStr, newMode)) {
        utils::printError("Invalid octal mode. Use exactly 3 digits 0-7 "
                           "(e.g. 754).");
        return false;
    }
    // Capture current mode for the log / comparison
    struct stat st;
    mode_t oldMode = 0;
    if (::lstat(path.c_str(), &st) == 0) oldMode = st.st_mode;
    std::string oldSym = utils::permissionToString(oldMode);
    std::string oldOct = utils::permissionToOctal(oldMode);

    // Show what we are about to do and ask for confirmation.
    std::cout << "  Current : " << oldSym << " (" << oldOct << ")\n"
              << "  New     : " << utils::permissionToString(newMode)
              << " (" << octalStr << ")\n";
    if (!utils::confirmAction("Apply this change?")) {
        utils::printInfo("Permission change cancelled.");
        logger.log("CHMOD", path, octalStr, "CANCELLED", "User declined");
        return false;
    }
    // chmod() on the path itself.  On a symlink, chmod() would follow the
    // link and modify the target — POSIX specifies that chmod on a
    // symlink follows it.  We warn about that explicitly.
    bool isLink = S_ISLNK(oldMode);
    if (isLink) {
        utils::printWarning("Path is a symbolic link. chmod() will modify "
                            "the TARGET, not the link itself.");
    }
    if (::chmod(path.c_str(), newMode) != 0) {
        utils::printError("chmod() failed: " + std::string(strerror(errno)));
        utils::printInfo("Hint: if you do not own this file you may need to "
                         "use sudo for this individual operation. "
                         "Do NOT run the whole application as root.");
        logger.log("CHMOD", path, octalStr, "FAILURE", strerror(errno));
        return false;
    }
    utils::printSuccess("Permissions changed.");
    logger.log("CHMOD", path, octalStr, "SUCCESS",
               oldOct + "->" + octalStr);
    return true;
}

// ---------------------------------------------------------------------
// Set permissions interactively using menus
// ---------------------------------------------------------------------
bool Permissions::setSymbolicMenu(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::exists(path)) {
        utils::printError("Path does not exist.");
        return false;
    }
    struct stat st;
    if (::lstat(path.c_str(), &st) != 0) {
        utils::printError("lstat() failed: " + std::string(strerror(errno)));
        return false;
    }
    mode_t oldMode = st.st_mode;
    mode_t newMode = oldMode & 0777;

    auto toggle = [](mode_t& m, mode_t bit, bool on) {
        if (on) m |= bit; else m &= ~bit;
    };
    auto ask = [](const std::string& who, const std::string& what,
                  bool cur) -> bool {
        std::cout << "  Allow " << what << " for " << who
                  << "? (currently " << (cur ? "Y" : "N") << ") (y/N): ";
        std::string ans;
        std::getline(std::cin, ans);
        return (!ans.empty() && (ans[0] == 'y' || ans[0] == 'Y'));
    };

    utils::printSubHeader("Build a new permission set");
    std::cout << "Current: " << utils::permissionToString(newMode)
              << " (" << utils::permissionToOctal(newMode) << ")\n\n";

    bool r, w, x;
    // Owner
    r = ask("owner", "read",   (newMode & S_IRUSR) != 0); toggle(newMode, S_IRUSR, r);
    w = ask("owner", "write",  (newMode & S_IWUSR) != 0); toggle(newMode, S_IWUSR, w);
    x = ask("owner", "execute",(newMode & S_IXUSR) != 0); toggle(newMode, S_IXUSR, x);
    // Group
    r = ask("group", "read",   (newMode & S_IRGRP) != 0); toggle(newMode, S_IRGRP, r);
    w = ask("group", "write",  (newMode & S_IWGRP) != 0); toggle(newMode, S_IWGRP, w);
    x = ask("group", "execute",(newMode & S_IXGRP) != 0); toggle(newMode, S_IXGRP, x);
    // Other
    r = ask("other", "read",   (newMode & S_IROTH) != 0); toggle(newMode, S_IROTH, r);
    w = ask("other", "write",  (newMode & S_IWOTH) != 0); toggle(newMode, S_IWOTH, w);
    x = ask("other", "execute",(newMode & S_IXOTH) != 0); toggle(newMode, S_IXOTH, x);

    std::cout << "\n  Old: " << utils::permissionToString(oldMode & 0777)
              << " (" << utils::permissionToOctal(oldMode) << ")\n"
              << "  New: " << utils::permissionToString(newMode)
              << " (" << utils::permissionToOctal(newMode) << ")\n";
    if (!utils::confirmAction("Apply?")) {
        utils::printInfo("Cancelled.");
        logger.log("CHMOD", path, "", "CANCELLED", "User declined");
        return false;
    }
    if (::chmod(path.c_str(), newMode) != 0) {
        utils::printError("chmod() failed: " + std::string(strerror(errno)));
        logger.log("CHMOD", path, "", "FAILURE", strerror(errno));
        return false;
    }
    utils::printSuccess("Permissions changed.");
    logger.log("CHMOD", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// Check access for the current user
// ---------------------------------------------------------------------
bool Permissions::checkAccess(const std::string& path) {
    if (!utils::isValidPath(path) || !fs::exists(path)) {
        utils::printError("Invalid or non-existent path.");
        return false;
    }
    bool r = (::access(path.c_str(), R_OK) == 0);
    bool w = (::access(path.c_str(), W_OK) == 0);
    bool x = (::access(path.c_str(), X_OK) == 0);
    utils::printSubHeader("Access check for current user");
    std::cout << "  Path      : " << path << "\n"
              << "  Readable  : " << (r ? "yes" : "no") << "\n"
              << "  Writable  : " << (w ? "yes" : "no") << "\n"
              << "  Executable: " << (x ? "yes" : "no") << "\n"
              << "  uid=" << ::getuid() << "  gid=" << ::getgid() << "\n";
    logger.log("ACCESS", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// Compare before/after — show old vs current
// ---------------------------------------------------------------------
bool Permissions::comparePermissions(const std::string& path) {
    if (!utils::isValidPath(path) || !fs::exists(path)) {
        utils::printError("Invalid or non-existent path.");
        return false;
    }
    struct stat st;
    if (::lstat(path.c_str(), &st) != 0) {
        utils::printError("lstat() failed.");
        return false;
    }
    std::string sym = utils::permissionToString(st.st_mode);
    std::string oct = utils::permissionToOctal(st.st_mode);
    std::cout << "Current permissions of " << path << ":\n"
              << "  Symbolic : " << sym << "\n"
              << "  Octal    : " << oct << "\n"
              << "(Compare with the 'before' values shown when you changed "
              << "permissions earlier in this session.)\n";
    return true;
}

// ---------------------------------------------------------------------
// Demonstration on a test file
// ---------------------------------------------------------------------
bool Permissions::runDemonstration() {
    utils::printHeader("PERMISSIONS DEMONSTRATION");
    std::cout << "This demo will create a temporary file 'demo_perm.txt' "
              << "in the current directory and cycle through a few "
              << "permission modes.\n\n";
    if (!utils::confirmAction("Proceed?")) {
        utils::printInfo("Demonstration cancelled.");
        return false;
    }
    std::string demoPath = "demo_perm.txt";
    // Create the file if it does not exist.
    if (!fs::exists(demoPath)) {
        std::ofstream ofs(demoPath);
        if (!ofs) {
            utils::printError("Could not create demo file.");
            return false;
        }
        ofs << "demo content\n";
        ofs.close();
    }
    auto showOne = [&](const std::string& label) {
        struct stat st;
        if (::lstat(demoPath.c_str(), &st) != 0) return;
        std::cout << "  [" << label << "] "
                  << utils::permissionToString(st.st_mode)
                  << " (" << utils::permissionToOctal(st.st_mode) << ")\n";
    };
    showOne("Original");
    auto apply = [&](mode_t m, const std::string& label) {
        if (::chmod(demoPath.c_str(), m) == 0) {
            showOne(label);
        } else {
            utils::printError("chmod failed for " + label + ": " +
                              std::string(strerror(errno)));
        }
    };
    apply(0644, "After  chmod 644 (rw-r--r--)  -- common text file");
    apply(0600, "After  chmod 600 (rw-------)  -- owner only");
    apply(0755, "After  chmod 755 (rwxr-xr-x)  -- executable + readable");
    apply(0000, "After  chmod 000 (----------)  -- no permissions");
    apply(0644, "Restored to 644");
    std::cout << "\nExplanation of common modes:\n"
              << "  644 = owner: rw,  group: r,   other: r     (text files)\n"
              << "  755 = owner: rwx, group: rx,  other: rx    (executables)\n"
              << "  600 = owner: rw,  group: none,other: none  (private files)\n"
              << "  000 = nobody has any permission\n"
              << "  777 = everybody can do everything (risky!)\n"
              << "\nNote: as the OWNER of demo_perm.txt you can change its "
              << "permissions. For files you do NOT own, you may need to use "
              << "sudo for that single chmod operation. Do NOT run the whole "
              << "application as root.\n";
    logger.log("PERM_DEMO", demoPath, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// Sub-menu
// ---------------------------------------------------------------------
void Permissions::showMenu() {
    while (true) {
        utils::printHeader("FILE PERMISSIONS");
        std::cout << "  1. View permissions of a file/directory\n"
                  << "  2. Set permissions (octal mode)\n"
                  << "  3. Set permissions (interactive menu)\n"
                  << "  4. Check read/write/execute access\n"
                  << "  5. Compare permissions\n"
                  << "  6. Run demonstration on a test file\n"
                  << "  0. Return to Main Menu\n\n";
        int choice = -1;
        if (!utils::getInt("Enter your choice", choice)) {
            if (utils::eofReached()) return;          // piped input ran out
            utils::printError("Invalid input.");
            utils::pauseScreen();
            continue;
        }
        std::string p1, p2;
        switch (choice) {
            case 1:
                p1 = utils::getInput("Enter path");
                viewPermissions(p1); break;
            case 2:
                p1 = utils::getInput("Enter path");
                p2 = utils::getInput("Enter octal mode (e.g. 754)");
                setOctalMode(p1, p2); break;
            case 3:
                p1 = utils::getInput("Enter path");
                setSymbolicMenu(p1); break;
            case 4:
                p1 = utils::getInput("Enter path");
                checkAccess(p1); break;
            case 5:
                p1 = utils::getInput("Enter path");
                comparePermissions(p1); break;
            case 6:
                runDemonstration(); break;
            case 0:
                return;
            default:
                utils::printError("Invalid choice. Please select 0-6.");
        }
        utils::pauseScreen();
    }
}
