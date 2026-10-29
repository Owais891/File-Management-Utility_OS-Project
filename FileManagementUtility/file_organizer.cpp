// file_organizer.cpp
// Implementation of the automatic file organizer.

#include "file_organizer.h"
#include "utils.h"
#include "activity_log.h"

#include <iostream>
#include <iomanip>
#include <set>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

static ActivityLog& logger = ActivityLog::getInstance();

FileOrganizer::FileOrganizer() = default;

// ---------------------------------------------------------------------
// Categorize a file extension into a folder name.
// ---------------------------------------------------------------------
std::string FileOrganizer::categoryFor(const std::string& ext) {
    std::string e = utils::toLower(ext);
    static const std::set<std::string> docs = {
        "txt","pdf","doc","docx","odt","rtf","md","csv","xls","xlsx","ppt","pptx"
    };
    static const std::set<std::string> images = {
        "jpg","jpeg","png","gif","bmp","svg","webp","tiff","ico","heic"
    };
    static const std::set<std::string> videos = {
        "mp4","mkv","avi","mov","webm","flv","wmv","mpeg","mpg","m4v"
    };
    static const std::set<std::string> audio = {
        "mp3","wav","flac","aac","ogg","m4a","wma","aiff"
    };
    static const std::set<std::string> archives = {
        "zip","rar","7z","tar","gz","bz2","xz","iso","tgz"
    };
    static const std::set<std::string> code = {
        "c","cpp","cc","h","hpp","py","java","js","ts","rs","go","rb",
        "php","sh","bash","pl","lua","kt","swift","sql","html","css","json","xml"
    };
    if (e.empty())             return "Others";
    if (docs.count(e))          return "Documents";
    if (images.count(e))       return "Images";
    if (videos.count(e))       return "Videos";
    if (audio.count(e))        return "Audio";
    if (archives.count(e))     return "Archives";
    if (code.count(e))         return "Code";
    return "Others";
}

// ---------------------------------------------------------------------
// Build a move plan without performing any move yet.
// ---------------------------------------------------------------------
std::vector<FileOrganizer::MovePlan>
FileOrganizer::buildPlan(const std::string& src, const std::string& dst) {
    std::vector<MovePlan> plan;
    if (!fs::is_directory(src)) {
        utils::printError("Source is not a directory.");
        return plan;
    }
    // Make dst absolute so we can later prevent "moving dst into itself".
    fs::path absDst = fs::weakly_canonical(fs::absolute(dst));
    fs::path absSrc = fs::weakly_canonical(fs::absolute(src));

    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(
                src,
                fs::directory_options::skip_permission_denied,
                ec);
         it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) { ec.clear(); continue; }
        const auto& e = *it;
        if (!e.is_regular_file()) continue;
        // Skip if the file is INSIDE the destination directory already.
        fs::path absFile = fs::weakly_canonical(e.path());
        if (absFile.string().rfind(absDst.string(), 0) == 0) continue;

        MovePlan p;
        p.source = e.path().string();
        p.extension = utils::getExtension(e.path().string());
        p.category = categoryFor(p.extension);
        p.destinationDir = (absDst / p.category).string();
        p.destinationPath =
            (fs::path(p.destinationDir) / e.path().filename()).string();
        plan.push_back(p);
    }
    return plan;
}

// ---------------------------------------------------------------------
// Preview the plan and confirm with the user.
// ---------------------------------------------------------------------
bool FileOrganizer::previewAndConfirm(const std::string& src,
                                       const std::string& dst) {
    if (!utils::isValidPath(src) || !utils::isValidPath(dst)) {
        utils::printError("Invalid path.");
        return false;
    }
    if (!fs::is_directory(src)) {
        utils::printError("Source directory does not exist or is not a dir.");
        return false;
    }
    // Refuse to organize the destination into itself.
    fs::path absSrc = fs::weakly_canonical(fs::absolute(src));
    fs::path absDst = fs::weakly_canonical(fs::absolute(dst));
    if (absSrc == absDst) {
        utils::printError("Source and destination are the same directory.");
        return false;
    }
    if (absDst.string().rfind(absSrc.string(), 0) == 0) {
        utils::printError("Destination is inside source. This would cause "
                           "infinite recursion. Choose a different destination.");
        return false;
    }

    auto plan = buildPlan(src, dst);
    if (plan.empty()) {
        utils::printInfo("No regular files to organize in source.");
        return false;
    }
    utils::printHeader("DRY-RUN PREVIEW");
    std::cout << "Source      : " << src << "\n"
              << "Destination : " << dst << "\n"
              << "Planned moves: " << plan.size() << "\n\n";
    std::cout << "  Category      Source\n";
    std::cout << "  ----------    --------------------------------------\n";
    for (const auto& p : plan) {
        std::cout << "  " << std::left << std::setw(12) << p.category
                  << "    " << p.source
                  << "\n        -> " << p.destinationPath << "\n";
    }
    if (!utils::confirmAction("\nProceed with these moves?")) {
        utils::printInfo("Organization cancelled by user.");
        logger.log("ORGANIZE_PREVIEW", src, dst, "CANCELLED", "User declined");
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------
// Actually perform the moves.
// ---------------------------------------------------------------------
bool FileOrganizer::organize(const std::string& src, const std::string& dst) {
    if (!previewAndConfirm(src, dst)) return false;
    auto plan = buildPlan(src, dst);   // rebuild after confirmation
    int ok = 0, fail = 0, skipped = 0;
    std::error_code ec;
    // Create the top-level destination first.
    fs::create_directories(dst, ec);
    for (const auto& p : plan) {
        fs::create_directories(p.destinationDir, ec);
        if (ec) {
            utils::printError("Could not create folder '" +
                               p.destinationDir + "': " + ec.message());
            logger.log("ORGANIZE", p.source, p.destinationDir, "FAILURE",
                       ec.message());
            ++fail; ec.clear();
            continue;
        }
        // Do NOT overwrite existing file at the destination silently.
        if (fs::exists(p.destinationPath)) {
            // Add a numeric suffix to keep both files.
            fs::path base = fs::path(p.destinationPath);
            std::string stem = base.stem().string();
            std::string ext = base.extension().string();
            fs::path parent = base.parent_path();
            int counter = 1;
            fs::path candidate;
            do {
                candidate = parent / (stem + " (" +
                            std::to_string(counter) + ")" + ext);
                ++counter;
            } while (fs::exists(candidate));
            std::error_code ec2;
            fs::rename(p.source, candidate, ec2);
            if (ec2) {
                utils::printError("Failed: " + p.source + " -> " +
                                  candidate.string() + " (" +
                                  ec2.message() + ")");
                logger.log("ORGANIZE", p.source, candidate.string(),
                           "FAILURE", ec2.message());
                ++fail;
            } else {
                utils::printSuccess("Renamed existing target: " +
                                    p.source + " -> " + candidate.string());
                logger.log("ORGANIZE", p.source, candidate.string(),
                           "SUCCESS", "renamed-existing");
                ++ok;
            }
            continue;
        }
        std::error_code ec3;
        fs::rename(p.source, p.destinationPath, ec3);
        if (ec3) {
            // Fall back to copy+remove
            ec3.clear();
            fs::copy_file(p.source, p.destinationPath,
                          fs::copy_options::overwrite_existing, ec3);
            if (ec3) {
                utils::printError("Failed: " + p.source + " -> " +
                                  p.destinationPath + " (" +
                                  ec3.message() + ")");
                logger.log("ORGANIZE", p.source, p.destinationPath,
                           "FAILURE", ec3.message());
                ++fail;
                continue;
            }
            ec3.clear();
            fs::remove(p.source, ec3);
            if (ec3) {
                utils::printWarning("Copied but could not remove source: " +
                                    p.source);
            }
        }
        utils::printSuccess("Moved " + p.source + " -> " + p.destinationPath);
        logger.log("ORGANIZE", p.source, p.destinationPath, "SUCCESS", "");
        ++ok;
    }
    utils::printSubHeader("Organization summary");
    std::cout << "  Total   : " << plan.size() << "\n"
              << "  Success : " << ok        << "\n"
              << "  Failed  : " << fail      << "\n"
              << "  Skipped : " << skipped   << "\n";
    return true;
}

// ---------------------------------------------------------------------
// Sub-menu
// ---------------------------------------------------------------------
void FileOrganizer::showMenu() {
    while (true) {
        utils::printHeader("FILE ORGANIZER");
        std::cout << "  1. Preview (dry-run) organization\n"
                  << "  2. Organize files now\n"
                  << "  0. Return to Main Menu\n\n";
        int choice = -1;
        if (!utils::getInt("Enter your choice", choice)) {
            if (utils::eofReached()) return;          // piped input ran out
            utils::printError("Invalid input.");
            utils::pauseScreen();
            continue;
        }
        if (choice == 0) return;
        if (choice < 1 || choice > 2) {
            utils::printError("Invalid choice.");
            utils::pauseScreen();
            continue;
        }
        std::string src = utils::getInput("Enter source directory");
        std::string dst = utils::getInput("Enter destination directory");
        if (choice == 1) {
            // Dry-run only — show preview and stop
            auto plan = buildPlan(src, dst);
            if (plan.empty()) {
                utils::printInfo("No files to organize.");
            } else {
                std::cout << "Plan: " << plan.size() << " move(s)\n";
                for (const auto& p : plan) {
                    std::cout << "  " << p.category << "  " << p.source
                              << " -> " << p.destinationPath << "\n";
                }
            }
        } else {
            organize(src, dst);
        }
        utils::pauseScreen();
    }
}
