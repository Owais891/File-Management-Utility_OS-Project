// duplicate_finder.cpp
// Implementation of duplicate-file detection.
//
// Algorithm:
//   1. Walk the directory recursively, collect (path, size) for every
//      regular file.
//   2. Group files by size — only files that share a size with at least
//      one other file are candidates for duplication.
//   3. For each candidate group, do a byte-by-byte comparison.  Two
//      files are duplicates if and only if every byte matches.
//   4. Report each group of identical files.
//
// We deliberately do NOT assume that equal name or equal size means the
// files are duplicates.  Only byte-level equality counts.

#include "duplicate_finder.h"
#include "utils.h"
#include "activity_log.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <filesystem>
#include <algorithm>
#include <cstring>

namespace fs = std::filesystem;

static ActivityLog& logger = ActivityLog::getInstance();

bool DuplicateFinder::readFileBytes(const std::string& path,
                                     std::vector<char>& out) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return false;
    ifs.seekg(0, std::ios::end);
    std::streamoff sz = ifs.tellg();
    if (sz < 0) return false;
    ifs.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(sz));
    if (sz > 0) ifs.read(out.data(), sz);
    return ifs.good() || ifs.eof();
}

bool DuplicateFinder::filesAreEqual(const std::string& a,
                                     const std::string& b) {
    std::ifstream fa(a, std::ios::binary);
    std::ifstream fb(b, std::ios::binary);
    if (!fa || !fb) return false;
    // Quick size check first (already known to match in our pipeline,
    // but cheap and safe).
    fa.seekg(0, std::ios::end); fb.seekg(0, std::ios::end);
    if (fa.tellg() != fb.tellg()) return false;
    fa.seekg(0, std::ios::beg);  fb.seekg(0, std::ios::beg);
    // Compare in chunks to limit memory use.
    static const std::streamsize BUF = 8192;
    char ba[BUF], bb[BUF];
    while (fa && fb) {
        fa.read(ba, BUF); fb.read(bb, BUF);
        std::streamsize ga = fa.gcount();
        std::streamsize gb = fb.gcount();
        if (ga != gb) return false;
        if (std::memcmp(ba, bb, static_cast<size_t>(ga)) != 0) return false;
        if (ga < BUF) break;
    }
    return true;
}

bool DuplicateFinder::findDuplicates(const std::string& dir) {
    if (!utils::isValidPath(dir) || !fs::is_directory(dir)) {
        utils::printError("Invalid or non-directory path.");
        return false;
    }
    // 1. Collect files
    std::vector<FileInfo> all;
    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(
                dir,
                fs::directory_options::skip_permission_denied,
                ec);
         it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) { ec.clear(); continue; }
        const auto& e = *it;
        if (e.is_regular_file() && !e.is_symlink()) {
            FileInfo fi;
            fi.path = e.path().string();
            std::error_code s;
            fi.size = static_cast<long long>(e.file_size(s));
            if (s) { s.clear(); continue; }
            all.push_back(fi);
        }
    }
    if (all.empty()) {
        utils::printInfo("No regular files in directory.");
        return true;
    }

    // 2. Group by size
    std::map<long long, std::vector<FileInfo>> bySize;
    for (const auto& f : all) bySize[f.size].push_back(f);

    // 3. Byte-compare within each candidate group.
    std::vector<DuplicateGroup> groups;
    for (auto& [size, files] : bySize) {
        if (files.size() < 2) continue;
        // visited[i] == true means already merged into another group
        std::vector<bool> visited(files.size(), false);
        for (size_t i = 0; i < files.size(); ++i) {
            if (visited[i]) continue;
            DuplicateGroup g;
            g.size = size;
            g.paths.push_back(files[i].path);
            visited[i] = true;
            for (size_t j = i + 1; j < files.size(); ++j) {
                if (visited[j]) continue;
                if (filesAreEqual(files[i].path, files[j].path)) {
                    g.paths.push_back(files[j].path);
                    visited[j] = true;
                }
            }
            if (g.paths.size() >= 2) groups.push_back(g);
        }
    }

    // 4. Report
    utils::printHeader("DUPLICATE FILE FINDER");
    std::cout << "Scanned directory : " << dir << "\n"
              << "Regular files      : " << all.size() << "\n"
              << "Duplicate groups   : " << groups.size() << "\n\n";

    long long wasted = 0;
    int totalDup = 0;
    for (size_t gi = 0; gi < groups.size(); ++gi) {
        const auto& g = groups[gi];
        std::cout << "Group " << (gi + 1) << " — " << g.paths.size()
                  << " copies, size = " << utils::formatSize(g.size)
                  << " each\n";
        for (size_t k = 0; k < g.paths.size(); ++k) {
            std::cout << "  " << (k+1) << ". " << g.paths[k] << "\n";
        }
        wasted += static_cast<long long>(g.paths.size() - 1) * g.size;
        totalDup += static_cast<int>(g.paths.size() - 1);
        std::cout << "\n";
    }
    if (groups.empty()) {
        utils::printInfo("No duplicate files found.");
    } else {
        std::cout << "Potential storage recoverable: "
                  << utils::formatSize(wasted) << " (" << wasted
                  << " bytes) by deleting " << totalDup
                  << " duplicate copy(ies).\n";
    }
    logger.log("DUP_SCAN", dir, "", "SUCCESS",
              "groups=" + std::to_string(groups.size()) +
              " wasted=" + std::to_string(wasted));
    return true;
}

bool DuplicateFinder::removeDuplicatesInteractive(const std::string& dir) {
    if (!utils::isValidPath(dir) || !fs::is_directory(dir)) {
        utils::printError("Invalid or non-directory path.");
        return false;
    }
    // Reuse findDuplicates' logic by re-scanning here.  For brevity we
    // repeat the analysis.
    std::vector<FileInfo> all;
    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(
                dir,
                fs::directory_options::skip_permission_denied,
                ec);
         it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) { ec.clear(); continue; }
        const auto& e = *it;
        if (e.is_regular_file() && !e.is_symlink()) {
            FileInfo fi;
            fi.path = e.path().string();
            std::error_code s;
            fi.size = static_cast<long long>(e.file_size(s));
            if (s) { s.clear(); continue; }
            all.push_back(fi);
        }
    }
    std::map<long long, std::vector<FileInfo>> bySize;
    for (const auto& f : all) bySize[f.size].push_back(f);
    std::vector<DuplicateGroup> groups;
    for (auto& [size, files] : bySize) {
        if (files.size() < 2) continue;
        std::vector<bool> visited(files.size(), false);
        for (size_t i = 0; i < files.size(); ++i) {
            if (visited[i]) continue;
            DuplicateGroup g;
            g.size = size;
            g.paths.push_back(files[i].path);
            visited[i] = true;
            for (size_t j = i + 1; j < files.size(); ++j) {
                if (visited[j]) continue;
                if (filesAreEqual(files[i].path, files[j].path)) {
                    g.paths.push_back(files[j].path);
                    visited[j] = true;
                }
            }
            if (g.paths.size() >= 2) groups.push_back(g);
        }
    }
    if (groups.empty()) {
        utils::printInfo("No duplicate files to remove.");
        return true;
    }
    std::cout << "Found " << groups.size() << " duplicate group(s).\n";
    if (!utils::confirmAction("For each group, you will be asked which "
                              "copy to KEEP. Continue?")) {
        utils::printInfo("Removal cancelled.");
        return false;
    }
    int removed = 0;
    long long recovered = 0;
    for (size_t gi = 0; gi < groups.size(); ++gi) {
        const auto& g = groups[gi];
        std::cout << "\nGroup " << (gi+1) << " (size = "
                  << utils::formatSize(g.size) << "):\n";
        for (size_t k = 0; k < g.paths.size(); ++k) {
            std::cout << "  " << (k+1) << ". " << g.paths[k] << "\n";
        }
        int keepIdx = -1;
        std::cout << "Enter the number of the copy to KEEP (1-"
                  << g.paths.size() << ") or 0 to skip this group: ";
        std::string s; std::getline(std::cin, s);
        try { keepIdx = std::stoi(s); }
        catch (...) { keepIdx = -1; }
        if (keepIdx == 0) { utils::printInfo("Group skipped."); continue; }
        if (keepIdx < 1 || keepIdx > static_cast<int>(g.paths.size())) {
            utils::printError("Invalid choice; group skipped.");
            continue;
        }
        // Delete every copy except keepIdx
        for (size_t k = 0; k < g.paths.size(); ++k) {
            if ((int)(k+1) == keepIdx) continue;
            if (!utils::confirmAction("Delete '" + g.paths[k] + "'?")) {
                utils::printInfo("Skipped.");
                continue;
            }
            std::error_code ec2;
            if (fs::remove(g.paths[k], ec2)) {
                utils::printSuccess("Deleted: " + g.paths[k]);
                logger.log("DUP_DELETE", g.paths[k], "", "SUCCESS", "");
                ++removed; recovered += g.size;
            } else {
                utils::printError("Failed: " + ec2.message());
                logger.log("DUP_DELETE", g.paths[k], "", "FAILURE", ec2.message());
            }
        }
    }
    std::cout << "\nSummary: removed " << removed
              << " duplicate(s), recovered ~"
              << utils::formatSize(recovered) << ".\n";
    return true;
}

void DuplicateFinder::showMenu() {
    while (true) {
        utils::printHeader("DUPLICATE FILE FINDER");
        std::cout << "  1. Find duplicate files (read-only)\n"
                  << "  2. Find and remove duplicates (interactive)\n"
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
        std::string d = utils::getInput("Enter directory to scan");
        if (choice == 1) findDuplicates(d);
        else             removeDuplicatesInteractive(d);
        utils::pauseScreen();
    }
}
