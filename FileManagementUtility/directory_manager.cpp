// directory_manager.cpp
// Implementation of directory operations.

#include "directory_manager.h"
#include "utils.h"
#include "activity_log.h"

#include <iostream>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <map>
#include <unistd.h>     // getcwd, chdir
#include <sys/stat.h>

namespace fs = std::filesystem;

static ActivityLog& logger = ActivityLog::getInstance();

// ---------------------------------------------------------------------
// 1. Create directory (with parents if needed)
// ---------------------------------------------------------------------
bool DirectoryManager::createDirectory(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (fs::exists(path)) {
        utils::printError("Path already exists.");
        logger.log("MKDIR", path, "", "FAILURE", "Already exists");
        return false;
    }
    std::error_code ec;
    if (!fs::create_directories(path, ec)) {
        utils::printError("Could not create directory: " + ec.message());
        logger.log("MKDIR", path, "", "FAILURE", ec.message());
        return false;
    }
    utils::printSuccess("Created directory: " + path);
    logger.log("MKDIR", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 2. List directory contents (single level)
// ---------------------------------------------------------------------
bool DirectoryManager::listDirectory(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::exists(path)) {
        utils::printError("Path does not exist.");
        return false;
    }
    if (!fs::is_directory(path)) {
        utils::printError("Path is not a directory.");
        return false;
    }
    std::error_code ec;
    std::vector<fs::directory_entry> entries;
    for (auto it = fs::directory_iterator(path, ec);
         it != fs::directory_iterator(); it.increment(ec)) {
        if (ec) break;  // skip unreadable entries
        entries.push_back(*it);
    }
    if (ec && entries.empty()) {
        utils::printError("Cannot read directory: " + ec.message());
        return false;
    }
    // Sort alphabetically (case-insensitive)
    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return utils::toLower(a.path().filename().string()) <
                         utils::toLower(b.path().filename().string());
              });

    utils::printSubHeader("Contents of " + path);
    std::cout << "Total entries: " << entries.size() << "\n\n";
    // Table header
    std::cout << "Type   Permissions  Size          Name\n";
    std::cout << "----   -----------  -----------   ----------------\n";
    for (const auto& e : entries) {
        std::string name = e.path().filename().string();
        std::string type;
        std::string perm = "----------";
        std::string sizeStr = "-";
        struct stat st;
        if (::lstat(e.path().c_str(), &st) == 0) {
            perm = utils::permissionToString(st.st_mode);
        }
        if (e.is_directory())          type = "DIR";
        else if (e.is_regular_file())  { type = "FILE"; sizeStr = utils::formatSize(e.file_size()); }
        else if (e.is_symlink())        type = "LINK";
        else                            type = "OTH";
        std::cout << type << "    " << perm << "  "
                  << std::left << std::setw(12) << sizeStr
                  << "  " << name << "\n";
    }
    logger.log("LIST_DIR", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 3. Rename directory
// ---------------------------------------------------------------------
bool DirectoryManager::renameDirectory(const std::string& oldPath,
                                       const std::string& newPath) {
    if (!utils::isValidPath(oldPath) || !utils::isValidPath(newPath)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::exists(oldPath) || !fs::is_directory(oldPath)) {
        utils::printError("Source directory does not exist.");
        return false;
    }
    if (fs::exists(newPath)) {
        if (!utils::confirmAction("Destination already exists. Overwrite?")) {
            utils::printInfo("Rename cancelled.");
            logger.log("RENAME_DIR", oldPath, newPath, "CANCELLED", "User declined");
            return false;
        }
    }
    std::error_code ec;
    fs::rename(oldPath, newPath, ec);
    if (ec) {
        utils::printError("Rename failed: " + ec.message());
        logger.log("RENAME_DIR", oldPath, newPath, "FAILURE", ec.message());
        return false;
    }
    utils::printSuccess("Renamed directory.");
    logger.log("RENAME_DIR", oldPath, newPath, "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 4. Remove empty directory
// ---------------------------------------------------------------------
bool DirectoryManager::removeEmptyDirectory(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::exists(path)) {
        utils::printError("Directory does not exist.");
        return false;
    }
    if (!fs::is_directory(path)) {
        utils::printError("Path is not a directory.");
        return false;
    }
    // Check if directory is non-empty
    std::error_code ec;
    auto it = fs::directory_iterator(path, ec);
    if (ec) {
        utils::printError("Cannot read directory: " + ec.message());
        return false;
    }
    bool isEmpty = (it == fs::directory_iterator());
    if (!isEmpty) {
        utils::printError("Directory is not empty. Remove its contents first.");
        logger.log("RMDIR", path, "", "FAILURE", "Not empty");
        return false;
    }
    if (!utils::confirmAction("Remove empty directory '" + path + "'?")) {
        utils::printInfo("Removal cancelled.");
        return false;
    }
    if (!fs::remove(path, ec)) {
        utils::printError("Remove failed: " + ec.message());
        logger.log("RMDIR", path, "", "FAILURE", ec.message());
        return false;
    }
    utils::printSuccess("Removed empty directory: " + path);
    logger.log("RMDIR", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 5. Show current working directory — POSIX getcwd()
// ---------------------------------------------------------------------
bool DirectoryManager::displayCurrentDirectory() {
    char buf[4096];
    if (!::getcwd(buf, sizeof(buf))) {
        utils::printError("getcwd() failed.");
        return false;
    }
    std::cout << "Current working directory: " << buf << "\n";
    logger.log("GETCWD", buf, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 6. Change directory — POSIX chdir()
// ---------------------------------------------------------------------
bool DirectoryManager::navigateDirectory(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::is_directory(path)) {
        utils::printError("Path is not a directory or does not exist.");
        return false;
    }
    if (::chdir(path.c_str()) != 0) {
        utils::printError("chdir() failed (permission denied?).");
        logger.log("CHDIR", path, "", "FAILURE", "chdir failed");
        return false;
    }
    char buf[4096];
    if (::getcwd(buf, sizeof(buf))) {
        utils::printSuccess("Changed to: " + std::string(buf));
    }
    logger.log("CHDIR", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 7. Display directory tree (recursive with depth limit)
// ---------------------------------------------------------------------
// Internal recursive helper — does NOT follow symlinks (avoids cycles).
static void printTree(const fs::path& dir, const std::string& prefix,
                      unsigned depth, unsigned maxDepth, int& fileCount,
                      int& dirCount, int& skipCount) {
    if (depth >= maxDepth) return;
    std::error_code ec;
    std::vector<fs::directory_entry> entries;
    for (auto it = fs::directory_iterator(dir, ec);
         it != fs::directory_iterator(); it.increment(ec)) {
        if (ec) { ++skipCount; break; }
        entries.push_back(*it);
    }
    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return utils::toLower(a.path().filename().string()) <
                         utils::toLower(b.path().filename().string());
              });
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& e = entries[i];
        bool last = (i == entries.size() - 1);
        std::string branch = last ? "└── " : "├── ";
        std::string name   = e.path().filename().string();
        std::string suffix;
        if (e.is_symlink()) suffix = " [symlink — not followed]";
        else if (e.is_directory()) { suffix = "/"; ++dirCount; }
        else if (e.is_regular_file()) { ++fileCount; }
        std::cout << prefix << branch << name << suffix << "\n";
        if (e.is_directory() && !e.is_symlink()) {
            std::string nextPrefix = prefix + (last ? "    " : "│   ");
            printTree(e.path(), nextPrefix, depth + 1, maxDepth,
                      fileCount, dirCount, skipCount);
        }
    }
}

bool DirectoryManager::displayTree(const std::string& path, unsigned maxDepth) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::is_directory(path)) {
        utils::printError("Path is not a directory.");
        return false;
    }
    utils::printSubHeader("Directory tree: " + path);
    std::cout << path << "/\n";
    int fileCount = 0, dirCount = 0, skipCount = 0;
    printTree(path, "", 0, maxDepth, fileCount, dirCount, skipCount);
    std::cout << "\nSummary: " << fileCount << " file(s), "
              << dirCount << " subdirectory(ies)";
    if (skipCount > 0)
        std::cout << ", " << skipCount << " entries skipped (permission)";
    std::cout << "\nMax depth: " << maxDepth << "\n";
    logger.log("TREE", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 8. Count entries
// ---------------------------------------------------------------------
bool DirectoryManager::countEntries(const std::string& path) {
    if (!utils::isValidPath(path) || !fs::is_directory(path)) {
        utils::printError("Invalid or non-directory path.");
        return false;
    }
    int files = 0, dirs = 0, links = 0, others = 0, hidden = 0;
    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(
                path,
                fs::directory_options::skip_permission_denied,
                ec);
         it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) { ec.clear(); continue; }
        const auto& e = *it;
        if (e.is_regular_file())      ++files;
        else if (e.is_directory())    ++dirs;
        else if (e.is_symlink())      ++links;
        else                          ++others;
        if (utils::isHidden(e.path().filename().string())) ++hidden;
    }
    utils::printSubHeader("Entry counts for " + path);
    std::cout << "  Regular files  : " << files  << "\n"
              << "  Directories    : " << dirs   << "\n"
              << "  Symbolic links : " << links  << "\n"
              << "  Other types    : " << others << "\n"
              << "  Hidden entries : " << hidden << "\n";
    logger.log("COUNT", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 9. Find hidden files/directories (single-level)
// ---------------------------------------------------------------------
bool DirectoryManager::findHidden(const std::string& path) {
    if (!utils::isValidPath(path) || !fs::is_directory(path)) {
        utils::printError("Invalid or non-directory path.");
        return false;
    }
    std::error_code ec;
    std::vector<fs::directory_entry> hidden;
    for (auto it = fs::directory_iterator(path, ec);
         it != fs::directory_iterator(); it.increment(ec)) {
        if (ec) break;
        if (utils::isHidden(it->path().filename().string()))
            hidden.push_back(*it);
    }
    utils::printSubHeader("Hidden entries in " + path);
    if (hidden.empty()) {
        std::cout << "  (no hidden entries)\n";
    } else {
        std::cout << "  Type   Permissions  Size          Name\n";
        std::cout << "  ----   -----------  -----------   ----------------\n";
        for (const auto& e : hidden) {
            std::string type, sizeStr = "-";
            struct stat st;
            std::string perm = "----------";
            if (::lstat(e.path().c_str(), &st) == 0)
                perm = utils::permissionToString(st.st_mode);
            if (e.is_directory())         type = "DIR";
            else if (e.is_regular_file()){ type = "FILE"; sizeStr = utils::formatSize(e.file_size()); }
            else if (e.is_symlink())      type = "LINK";
            else                          type = "OTH";
            std::cout << "  " << type << "   " << perm << "  "
                      << std::left << std::setw(12) << sizeStr
                      << "  " << e.path().filename().string() << "\n";
        }
    }
    std::cout << "\nNote: a hidden filename (starting with '.') does NOT "
              << "provide security or encryption.\n";
    logger.log("HIDDEN_LIST", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 10. Directory statistics (reuses recursive iterator)
// ---------------------------------------------------------------------
bool DirectoryManager::calculateStats(const std::string& path) {
    if (!utils::isValidPath(path) || !fs::is_directory(path)) {
        utils::printError("Invalid or non-directory path.");
        return false;
    }
    int files = 0, dirs = 0, links = 0, hidden = 0, errors = 0;
    long long totalBytes = 0;
    long long largestSize = -1, smallestSize = -1;
    std::string largestName, smallestName;
    std::map<std::string, int> byExtension;
    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(
                path,
                fs::directory_options::skip_permission_denied,
                ec);
         it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) { ec.clear(); ++errors; continue; }
        const auto& e = *it;
        if (e.is_directory()) ++dirs;
        else if (e.is_regular_file()) {
            ++files;
            long long sz = static_cast<long long>(e.file_size(ec));
            if (ec) { ec.clear(); ++errors; continue; }
            totalBytes += sz;
            if (largestSize < 0 || sz > largestSize) {
                largestSize = sz; largestName = e.path().string();
            }
            if (smallestSize < 0 || sz < smallestSize) {
                smallestSize = sz; smallestName = e.path().string();
            }
            std::string ext = utils::getExtension(e.path().string());
            if (ext.empty()) ext = "(no extension)";
            byExtension[utils::toLower(ext)]++;
        } else if (e.is_symlink()) {
            // We do NOT follow the symlink, so its target is not counted
            // again here. This avoids double-counting.
            ++links;
        }
        if (utils::isHidden(e.path().filename().string())) ++hidden;
    }
    double avg = files > 0 ? static_cast<double>(totalBytes) / files : 0.0;

    utils::printSubHeader("Directory statistics: " + path);
    std::cout << "  Regular files        : " << files << "\n"
              << "  Directories          : " << dirs  << "\n"
              << "  Symbolic links       : " << links << "\n"
              << "  Hidden entries       : " << hidden << "\n"
              << "  Total bytes (regular): " << totalBytes
              << "  (" << utils::formatSize(totalBytes) << ")\n"
              << "  Largest file         : "
              << (largestSize < 0 ? "(none)" :
                  largestName + " (" + utils::formatSize(largestSize) + ")") << "\n"
              << "  Smallest file        : "
              << (smallestSize < 0 ? "(none)" :
                  smallestName + " (" + utils::formatSize(smallestSize) + ")") << "\n"
              << "  Average file size    : " << utils::formatSize(static_cast<long long>(avg)) << "\n"
              << "  Skipped (errors)     : " << errors << "\n\n";

    std::cout << "  Files grouped by extension:\n";
    for (const auto& kv : byExtension) {
        std::cout << "    ." << std::left << std::setw(10) << kv.first
                  << " : " << kv.second << "\n";
    }
    std::cout << "\nNote: 'logical' file sizes are reported, which may differ "
              << "from the actual disk space consumed (block allocation).\n"
              << "Symbolic-link target sizes are NOT included here.\n";
    logger.log("DIR_STATS", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// Sub-menu
// ---------------------------------------------------------------------
void DirectoryManager::showMenu() {
    while (true) {
        utils::printHeader("DIRECTORY MANAGEMENT");
        std::cout << "  1. Create a directory\n"
                  << "  2. List directory contents\n"
                  << "  3. Rename a directory\n"
                  << "  4. Remove an empty directory\n"
                  << "  5. Show current working directory\n"
                  << "  6. Navigate to another directory\n"
                  << "  7. Display directory tree\n"
                  << "  8. Count files and subdirectories\n"
                  << "  9. Find hidden files and directories\n"
                  << " 10. Calculate directory statistics\n"
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
                p1 = utils::getInput("Enter new directory path");
                createDirectory(p1); break;
            case 2:
                p1 = utils::getInput("Enter directory path");
                listDirectory(p1); break;
            case 3:
                p1 = utils::getInput("Enter current directory path");
                p2 = utils::getInput("Enter new directory path");
                renameDirectory(p1, p2); break;
            case 4:
                p1 = utils::getInput("Enter empty directory path to remove");
                removeEmptyDirectory(p1); break;
            case 5:
                displayCurrentDirectory(); break;
            case 6:
                p1 = utils::getInput("Enter directory to navigate to");
                navigateDirectory(p1); break;
            case 7:
                p1 = utils::getInput("Enter directory path");
                displayTree(p1); break;
            case 8:
                p1 = utils::getInput("Enter directory path");
                countEntries(p1); break;
            case 9:
                p1 = utils::getInput("Enter directory path");
                findHidden(p1); break;
            case 10:
                p1 = utils::getInput("Enter directory path");
                calculateStats(p1); break;
            case 0:
                return;
            default:
                utils::printError("Invalid choice. Please select 0-10.");
        }
        utils::pauseScreen();
    }
}
