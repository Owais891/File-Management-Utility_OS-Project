// file_search.cpp
// Implementation of recursive file-search.

#include "file_search.h"
#include "utils.h"
#include "activity_log.h"

#include <iostream>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

static ActivityLog& logger = ActivityLog::getInstance();

void FileSearch::walk(const std::string& dir,
                      const std::function<bool(const fs::directory_entry&)>& pred,
                      std::vector<Result>& out) {
    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(
                dir,
                fs::directory_options::skip_permission_denied,
                ec);
         it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) { ec.clear(); continue; }
        const auto& e = *it;
        if (e.is_regular_file() && pred(e)) {
            Result r;
            r.path = e.path().string();
            r.name = e.path().filename().string();
            std::error_code s;
            r.size = static_cast<long long>(e.file_size(s));
            if (s) r.size = -1;
            out.push_back(r);
        }
    }
}

void FileSearch::displayResults(const std::vector<Result>& r) {
    if (r.empty()) {
        utils::printInfo("No matches found.");
        return;
    }
    utils::printSubHeader("Search results (" + std::to_string(r.size()) +
                          " match(es))");
    std::cout << "  #    Size          Path\n";
    std::cout << "  ---  -----------   --------------------------------\n";
    for (size_t i = 0; i < r.size(); ++i) {
        std::cout << "  " << std::left << std::setw(4) << (i + 1) << " "
                  << std::setw(12)
                  << (r[i].size < 0 ? "n/a" : utils::formatSize(r[i].size))
                  << "  " << r[i].path << "\n";
    }
    std::cout << "\nTotal matches: " << r.size() << "\n";
}

bool FileSearch::searchByName(const std::string& dir, const std::string& name) {
    if (!utils::isValidPath(dir) || !fs::is_directory(dir)) {
        utils::printError("Invalid or non-directory path.");
        return false;
    }
    std::string lname = utils::toLower(name);
    std::vector<Result> r;
    walk(dir,
         [&](const fs::directory_entry& e) {
             return utils::toLower(e.path().filename().string()) == lname;
         },
         r);
    displayResults(r);
    logger.log("SEARCH_NAME", dir, name,
               r.empty() ? "NO_MATCH" : "MATCH", "");
    return !r.empty();
}

bool FileSearch::searchByPartial(const std::string& dir,
                                 const std::string& partial) {
    if (!utils::isValidPath(dir) || !fs::is_directory(dir)) {
        utils::printError("Invalid or non-directory path.");
        return false;
    }
    std::string lpartial = utils::toLower(partial);
    std::vector<Result> r;
    walk(dir,
         [&](const fs::directory_entry& e) {
             return utils::toLower(e.path().filename().string())
                        .find(lpartial) != std::string::npos;
         },
         r);
    displayResults(r);
    logger.log("SEARCH_PARTIAL", dir, partial,
               r.empty() ? "NO_MATCH" : "MATCH", "");
    return !r.empty();
}

bool FileSearch::searchByExtension(const std::string& dir,
                                   const std::string& ext) {
    if (!utils::isValidPath(dir) || !fs::is_directory(dir)) {
        utils::printError("Invalid or non-directory path.");
        return false;
    }
    // Normalize: strip leading dot if user typed ".pdf"
    std::string lex = utils::toLower(ext);
    if (!lex.empty() && lex[0] == '.') lex = lex.substr(1);
    if (lex.empty()) {
        utils::printError("Empty extension.");
        return false;
    }
    std::vector<Result> r;
    walk(dir,
         [&](const fs::directory_entry& e) {
             std::string e_ext = utils::toLower(
                 e.path().extension().string());
             if (!e_ext.empty() && e_ext[0] == '.') e_ext = e_ext.substr(1);
             return e_ext == lex;
         },
         r);
    displayResults(r);
    logger.log("SEARCH_EXT", dir, lex,
               r.empty() ? "NO_MATCH" : "MATCH", "");
    return !r.empty();
}

void FileSearch::showMenu() {
    while (true) {
        utils::printHeader("FILE SEARCH");
        std::cout << "  1. Search by exact filename\n"
                  << "  2. Search by partial filename (substring)\n"
                  << "  3. Search by file extension\n"
                  << "  0. Return to Main Menu\n\n";
        int choice = -1;
        if (!utils::getInt("Enter your choice", choice)) {
            if (utils::eofReached()) return;          // piped input ran out
            utils::printError("Invalid input.");
            utils::pauseScreen();
            continue;
        }
        if (choice == 0) return;
        if (choice < 1 || choice > 3) {
            utils::printError("Invalid choice.");
            utils::pauseScreen();
            continue;
        }
        std::string dir = utils::getInput("Enter starting directory");
        if (choice == 1) {
            std::string n = utils::getInput("Enter exact filename (e.g. report.txt)");
            searchByName(dir, n);
        } else if (choice == 2) {
            std::string n = utils::getInput("Enter partial filename");
            searchByPartial(dir, n);
        } else {
            std::string n = utils::getInput("Enter extension (e.g. pdf)");
            searchByExtension(dir, n);
        }
        utils::pauseScreen();
    }
}
