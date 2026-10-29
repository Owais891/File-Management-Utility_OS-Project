// file_search.h
// Recursive file-search subsystem.
//
// Operating System Concepts Demonstrated:
//   - Recursive directory traversal using std::filesystem.
//   - Case-insensitive string comparison.
//   - Safe handling of permission errors via directory_options.

#ifndef FILE_SEARCH_H
#define FILE_SEARCH_H

#include <string>
#include <vector>
#include <filesystem>
#include <functional>

class FileSearch {
public:
    struct Result {
        std::string path;
        std::string name;
        long long    size;
    };

    // Search by exact (case-insensitive) filename.
    bool searchByName(const std::string& dir, const std::string& name);
    // Search by partial filename substring (case-insensitive).
    bool searchByPartial(const std::string& dir, const std::string& partial);
    // Search by extension (e.g. "pdf").  Case-insensitive.
    bool searchByExtension(const std::string& dir, const std::string& ext);

    // Interactive sub-menu.
    void showMenu();
private:
    // Internal: walks `dir` recursively, calling predicate to decide
    // which entries are matches.
    void walk(const std::string& dir,
              const std::function<bool(const std::filesystem::directory_entry&)>& pred,
              std::vector<Result>& out);
    void displayResults(const std::vector<Result>& r);
};

#endif // FILE_SEARCH_H
