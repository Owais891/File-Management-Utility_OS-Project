// main.cpp
// Entry point and top-level menu for the File Management Utility.
//
// This file wires together every module (file_manager, directory_manager,
// file_search, metadata, permissions, file_organizer, duplicate_finder,
// activity_log) into a single menu-driven CLI.
//
// Operating System Concepts Demonstrated (project-wide):
//   - File-system operations via C++17 <filesystem>.
//   - POSIX system calls: stat(), lstat(), chmod(), access(),
//     getcwd(), chdir(), getuid(), getgid(), readlink().
//   - User and group ownership inspection.
//   - Symbolic links — recognized, not silently followed.
//   - Linux file permissions (symbolic + octal).
//   - Robust error handling at the OS boundary.

#include "utils.h"
#include "activity_log.h"
#include "file_manager.h"
#include "directory_manager.h"
#include "file_search.h"
#include "metadata.h"
#include "permissions.h"
#include "file_organizer.h"
#include "duplicate_finder.h"

#include <iostream>
#include <string>

// Make logger visible everywhere in main.cpp
static ActivityLog& logger = ActivityLog::getInstance();

// Forward declarations of menu handlers
static void showMainMenu();
static void showHiddenFilesMenu();

// ---------------------------------------------------------------------
// Hidden-files menu — reuses DirectoryManager::findHidden
// (We expose it as its own top-level entry per the project spec.)
// ---------------------------------------------------------------------
static void showHiddenFilesMenu() {
    while (true) {
        utils::printHeader("HIDDEN FILES & DIRECTORIES");
        std::cout << "  1. Show hidden entries in a directory\n"
                  << "  0. Return to Main Menu\n\n";
        int c = -1;
        if (!utils::getInt("Enter your choice", c)) {
            if (utils::eofReached()) return;          // piped input ran out
            utils::printError("Invalid input.");
            utils::pauseScreen();
            continue;
        }
        if (c == 0) return;
        if (c == 1) {
            std::string d = utils::getInput("Enter directory path");
            DirectoryManager dm;
            dm.findHidden(d);
        } else {
            utils::printError("Invalid choice.");
        }
        utils::pauseScreen();
    }
}

// ---------------------------------------------------------------------
// Main menu
// ---------------------------------------------------------------------
static void showMainMenu() {
    FileManager        fm;
    DirectoryManager  dm;
    FileSearch        fs;
    Metadata          md;
    Permissions       pm;
    FileOrganizer     fo;
    DuplicateFinder   df;

    while (true) {
        utils::clearScreen();
        std::string bar(60, '=');
        std::cout << utils::CYAN << utils::BOLD;
        std::cout << "\n" << bar << "\n";
        std::cout << "          FILE MANAGEMENT UTILITY\n";
        std::cout << bar << utils::RESET << utils::BOLD << "\n\n";
        std::cout << utils::RESET;
        std::cout << "  1.  File Management\n"
                  << "  2.  Directory Management\n"
                  << "  3.  Search Files\n"
                  << "  4.  File Metadata\n"
                  << "  5.  File Permissions\n"
                  << "  6.  Directory Size Analyzer\n"
                  << "  7.  File Organizer\n"
                  << "  8.  Duplicate File Finder\n"
                  << "  9.  Hidden Files and Directories\n"
                  << " 10.  Activity Log\n"
                  << "  0.  Exit\n\n";
        int choice = -1;
        if (!utils::getInt("Enter your choice", choice)) {
            if (utils::eofReached()) return;          // piped input ran out
            utils::printError("Invalid input. Please enter a number.");
            utils::pauseScreen();
            continue;
        }
        switch (choice) {
            case 1:  fm.showMenu();   break;
            case 2:  dm.showMenu();   break;
            case 3:  fs.showMenu();   break;
            case 4:  md.showMenu();   break;
            case 5:  pm.showMenu();   break;
            case 6:  {
                // Directory Size Analyzer — reuses DirectoryManager
                utils::printHeader("DIRECTORY SIZE ANALYZER");
                std::string d = utils::getInput("Enter directory path");
                dm.calculateStats(d);
                utils::pauseScreen();
                break;
            }
            case 7:  fo.showMenu();   break;
            case 8:  df.showMenu();   break;
            case 9:  showHiddenFilesMenu(); break;
            case 10: {
                int n = 20;
                if (!utils::getInt("How many latest entries to show (default 20)", n)
                    || n <= 0) n = 20;
                logger.viewLatest(n);
                utils::pauseScreen();
                break;
            }
            case 0:
                utils::printInfo("Thank you for using File Management Utility.");
                logger.log("EXIT", "", "", "SUCCESS", "");
                return;
            default:
                utils::printError("Invalid choice. Please select 0-10.");
                utils::pauseScreen();
        }
    }
}

// ---------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------
int main() {
    showMainMenu();
    return 0;
}
