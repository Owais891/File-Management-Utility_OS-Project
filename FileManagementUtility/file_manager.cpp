// file_manager.cpp
// Implementation of file operations.
//
// Key safety rules followed here:
//   * We never call system().  All file operations use C++17 std::filesystem
//     and <fstream>.
//   * Copy / move / rename refuse to overwrite an existing destination
//     unless the user explicitly confirms.
//   * Delete requires explicit confirmation.
//   * All operations are logged via the ActivityLog singleton.

#include "file_manager.h"
#include "utils.h"
#include "activity_log.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <limits>

namespace fs = std::filesystem;

static ActivityLog& logger = ActivityLog::getInstance();

// ---------------------------------------------------------------------
// 1. Create file
// ---------------------------------------------------------------------
bool FileManager::createFile(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        logger.log("CREATE_FILE", path, "", "FAILURE", "Invalid path");
        return false;
    }
    // We refuse to silently overwrite an existing file.
    if (fs::exists(path)) {
        utils::printError("File already exists. Use Append or another path.");
        logger.log("CREATE_FILE", path, "", "FAILURE", "Already exists");
        return false;
    }
    std::ofstream ofs(path);
    if (!ofs) {
        utils::printError("Could not create file. Check permissions/path.");
        logger.log("CREATE_FILE", path, "", "FAILURE", "Open failed");
        return false;
    }
    ofs.close();
    utils::printSuccess("Created empty file: " + path);
    logger.log("CREATE_FILE", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 2. Read file
// ---------------------------------------------------------------------
bool FileManager::readFile(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        logger.log("READ_FILE", path, "", "FAILURE", "Invalid path");
        return false;
    }
    if (!fs::exists(path)) {
        utils::printError("File does not exist.");
        logger.log("READ_FILE", path, "", "FAILURE", "Not found");
        return false;
    }
    if (fs::is_directory(path)) {
        utils::printError("Path points to a directory, not a regular file.");
        logger.log("READ_FILE", path, "", "FAILURE", "Is a directory");
        return false;
    }
    // Use binary mode to avoid surprises; then print as text.  We also
    // limit the size we display so we don't dump a 1GB ISO to the screen.
    auto size = fs::file_size(path);
    const std::uintmax_t LIMIT = 1 * 1024 * 1024; // 1 MB
    if (size > LIMIT) {
        utils::printWarning("File is larger than 1 MB; only the first 1 MB "
                            "will be displayed.");
        utils::printInfo("Note: this viewer is designed for plain-text files. "
                         "Binary files (PDF, JPG, ZIP, etc.) may print garbage "
                         "characters on the terminal.");
    }

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        utils::printError("Could not open file (permission denied?).");
        logger.log("READ_FILE", path, "", "FAILURE", "Open failed");
        return false;
    }
    utils::printSubHeader("Contents of " + path);
    // Read in chunks
    const std::streamsize CHUNK = 4096;
    std::streamsize remaining = static_cast<std::streamsize>(
        std::min<std::uintmax_t>(size, LIMIT));
    char buf[CHUNK];
    bool binaryDetected = false;
    while (remaining > 0 && ifs) {
        std::streamsize want = std::min(CHUNK, remaining);
        ifs.read(buf, want);
        std::streamsize got = ifs.gcount();
        if (got <= 0) break;
        // Detect binary by looking for NUL bytes; if found, warn once.
        for (std::streamsize i = 0; i < got && !binaryDetected; ++i) {
            if (buf[i] == 0) binaryDetected = true;
        }
        std::cout.write(buf, got);
        remaining -= got;
    }
    std::cout << "\n";
    if (binaryDetected) {
        utils::printWarning("This file contains NUL bytes — it may be a "
                            "binary file. Text rendering may look corrupt.");
    }
    logger.log("READ_FILE", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 3. Append text
// ---------------------------------------------------------------------
bool FileManager::appendToFile(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        logger.log("APPEND_FILE", path, "", "FAILURE", "Invalid path");
        return false;
    }
    if (!fs::exists(path)) {
        utils::printError("File does not exist. Create it first.");
        logger.log("APPEND_FILE", path, "", "FAILURE", "Not found");
        return false;
    }
    if (fs::is_directory(path)) {
        utils::printError("Path is a directory, not a file.");
        return false;
    }
    std::string text = utils::getInput("Enter text to append (single line)");
    if (text.empty()) {
        utils::printWarning("Empty input; nothing appended.");
        return false;
    }
    // Open for append (ios::app seeks to end before every write).
    std::ofstream ofs(path, std::ios::app);
    if (!ofs) {
        utils::printError("Could not open file for writing.");
        logger.log("APPEND_FILE", path, "", "FAILURE", "Open failed");
        return false;
    }
    ofs << text << "\n";
    ofs.close();
    utils::printSuccess("Text appended successfully.");
    logger.log("APPEND_FILE", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 4. Copy file
// ---------------------------------------------------------------------
bool FileManager::copyFile(const std::string& src, const std::string& dst) {
    if (!utils::isValidPath(src) || !utils::isValidPath(dst)) {
        utils::printError("Invalid source or destination path.");
        return false;
    }
    if (!fs::exists(src)) {
        utils::printError("Source file does not exist.");
        logger.log("COPY_FILE", src, dst, "FAILURE", "Source not found");
        return false;
    }
    if (fs::is_directory(src)) {
        utils::printError("Source is a directory. Use Directory Management.");
        logger.log("COPY_FILE", src, dst, "FAILURE", "Source is directory");
        return false;
    }
    // If destination already exists, ask before overwriting.
    if (fs::exists(dst)) {
        if (!utils::confirmAction("Destination exists. Overwrite?")) {
            utils::printInfo("Copy cancelled by user.");
            logger.log("COPY_FILE", src, dst, "CANCELLED", "User declined overwrite");
            return false;
        }
    }
    std::error_code ec;
    fs::copy_file(src, dst,
                  fs::copy_options::overwrite_existing, ec);
    if (ec) {
        utils::printError("Copy failed: " + ec.message());
        logger.log("COPY_FILE", src, dst, "FAILURE", ec.message());
        return false;
    }
    utils::printSuccess("Copied '" + src + "' -> '" + dst + "'");
    logger.log("COPY_FILE", src, dst, "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 5. Move file
// ---------------------------------------------------------------------
bool FileManager::moveFile(const std::string& src, const std::string& dst) {
    if (!utils::isValidPath(src) || !utils::isValidPath(dst)) {
        utils::printError("Invalid source or destination path.");
        return false;
    }
    if (!fs::exists(src)) {
        utils::printError("Source file does not exist.");
        logger.log("MOVE_FILE", src, dst, "FAILURE", "Source not found");
        return false;
    }
    if (fs::is_directory(src)) {
        utils::printError("Source is a directory, not a file.");
        return false;
    }
    if (fs::exists(dst)) {
        if (!utils::confirmAction("Destination exists. Overwrite?")) {
            utils::printInfo("Move cancelled by user.");
            logger.log("MOVE_FILE", src, dst, "CANCELLED", "User declined");
            return false;
        }
    }
    std::error_code ec;
    fs::rename(src, dst, ec); // atomic on the same filesystem
    if (ec) {
        // Fall back to copy + remove
        ec.clear();
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            utils::printError("Move failed: " + ec.message());
            logger.log("MOVE_FILE", src, dst, "FAILURE", ec.message());
            return false;
        }
        ec.clear();
        fs::remove(src, ec);
        if (ec) {
            utils::printWarning("Copied to destination but could not delete source.");
            logger.log("MOVE_FILE", src, dst, "PARTIAL", ec.message());
            return true;
        }
    }
    utils::printSuccess("Moved '" + src + "' -> '" + dst + "'");
    logger.log("MOVE_FILE", src, dst, "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 6. Rename file
// ---------------------------------------------------------------------
bool FileManager::renameFile(const std::string& oldPath,
                              const std::string& newPath) {
    if (!utils::isValidPath(oldPath) || !utils::isValidPath(newPath)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::exists(oldPath)) {
        utils::printError("Source file does not exist.");
        logger.log("RENAME_FILE", oldPath, newPath, "FAILURE", "Source not found");
        return false;
    }
    if (fs::exists(newPath)) {
        if (!utils::confirmAction("A file with the new name already exists. Overwrite?")) {
            utils::printInfo("Rename cancelled.");
            logger.log("RENAME_FILE", oldPath, newPath, "CANCELLED", "User declined");
            return false;
        }
    }
    std::error_code ec;
    fs::rename(oldPath, newPath, ec);
    if (ec) {
        utils::printError("Rename failed: " + ec.message());
        logger.log("RENAME_FILE", oldPath, newPath, "FAILURE", ec.message());
        return false;
    }
    utils::printSuccess("Renamed '" + oldPath + "' -> '" + newPath + "'");
    logger.log("RENAME_FILE", oldPath, newPath, "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 7. Delete file
// ---------------------------------------------------------------------
bool FileManager::deleteFile(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::exists(path)) {
        utils::printError("File does not exist.");
        logger.log("DELETE_FILE", path, "", "FAILURE", "Not found");
        return false;
    }
    if (fs::is_directory(path)) {
        utils::printError("Path is a directory. Use Directory Management to remove.");
        return false;
    }
    if (!utils::confirmAction("Are you sure you want to delete '" + path + "'?")) {
        utils::printInfo("Deletion cancelled.");
        logger.log("DELETE_FILE", path, "", "CANCELLED", "User declined");
        return false;
    }
    std::error_code ec;
    if (!fs::remove(path, ec)) {
        utils::printError("Delete failed: " + ec.message());
        logger.log("DELETE_FILE", path, "", "FAILURE", ec.message());
        return false;
    }
    utils::printSuccess("Deleted file: " + path);
    logger.log("DELETE_FILE", path, "", "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 8. File exists
// ---------------------------------------------------------------------
bool FileManager::fileExists(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    bool ex = fs::exists(path);
    if (ex) {
        utils::printSuccess("Path exists: " + path);
        if (fs::is_regular_file(path))
            utils::printInfo("Type: regular file");
        else if (fs::is_directory(path))
            utils::printInfo("Type: directory");
        else if (fs::is_symlink(path))
            utils::printInfo("Type: symbolic link");
        else
            utils::printInfo("Type: other");
    } else {
        utils::printWarning("Path does not exist: " + path);
    }
    logger.log("CHECK_EXISTS", path, "", ex ? "EXISTS" : "NOT_FOUND", "");
    return ex;
}

// ---------------------------------------------------------------------
// 9. Absolute path
// ---------------------------------------------------------------------
bool FileManager::displayAbsolutePath(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::exists(path)) {
        utils::printWarning("Path does not exist on disk. "
                            "Showing lexical absolute path anyway.");
    }
    std::error_code ec;
    // Use weakly_canonical to resolve a (possibly non-existent) path to
    // an absolute form without throwing.
    fs::path abs = fs::weakly_canonical(fs::absolute(path, ec));
    if (ec) {
        utils::printError("Could not resolve absolute path: " + ec.message());
        return false;
    }
    std::cout << "Absolute path: " << abs.string() << "\n";
    logger.log("ABSOLUTE_PATH", path, abs.string(), "SUCCESS", "");
    return true;
}

// ---------------------------------------------------------------------
// 10. Display extension
// ---------------------------------------------------------------------
bool FileManager::displayExtension(const std::string& path) {
    if (!utils::isValidPath(path)) {
        utils::printError("Invalid path.");
        return false;
    }
    std::string ext = utils::getExtension(path);
    if (ext.empty()) {
        std::cout << "No extension (or filename ends with '.').\n";
    } else {
        std::cout << "Extension: ." << ext << "\n";
    }
    logger.log("DISPLAY_EXTENSION", path, "", "SUCCESS", ext);
    return true;
}

// ---------------------------------------------------------------------
// Sub-menu
// ---------------------------------------------------------------------
void FileManager::showMenu() {
    while (true) {
        utils::printHeader("FILE MANAGEMENT");
        std::cout << "  1. Create a new text file\n"
                  << "  2. Read a text file\n"
                  << "  3. Append text to a file\n"
                  << "  4. Copy a file\n"
                  << "  5. Move a file\n"
                  << "  6. Rename a file\n"
                  << "  7. Delete a file\n"
                  << "  8. Check if a file exists\n"
                  << "  9. Display absolute path\n"
                  << " 10. Display file extension\n"
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
                p1 = utils::getInput("Enter new file path");
                createFile(p1); break;
            case 2:
                p1 = utils::getInput("Enter file path to read");
                readFile(p1); break;
            case 3:
                p1 = utils::getInput("Enter file path to append to");
                appendToFile(p1); break;
            case 4:
                p1 = utils::getInput("Enter source file path");
                p2 = utils::getInput("Enter destination file path");
                copyFile(p1, p2); break;
            case 5:
                p1 = utils::getInput("Enter source file path");
                p2 = utils::getInput("Enter destination file path");
                moveFile(p1, p2); break;
            case 6:
                p1 = utils::getInput("Enter current file path");
                p2 = utils::getInput("Enter new file name/path");
                renameFile(p1, p2); break;
            case 7:
                p1 = utils::getInput("Enter file path to delete");
                deleteFile(p1); break;
            case 8:
                p1 = utils::getInput("Enter path to check");
                fileExists(p1); break;
            case 9:
                p1 = utils::getInput("Enter path");
                displayAbsolutePath(p1); break;
            case 10:
                p1 = utils::getInput("Enter path");
                displayExtension(p1); break;
            case 0:
                return;
            default:
                utils::printError("Invalid choice. Please select 0-10.");
        }
        utils::pauseScreen();
    }
}
