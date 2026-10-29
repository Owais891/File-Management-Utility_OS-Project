// duplicate_finder.h
// Duplicate-file finder.
//
// Operating System Concepts Demonstrated:
//   - Two-stage duplicate detection: group by size, then byte-compare.
//   - Direct read of file bytes via <fstream>.
//   - Avoidance of external hashing libraries (per project requirement).

#ifndef DUPLICATE_FINDER_H
#define DUPLICATE_FINDER_H

#include <string>
#include <vector>
#include <filesystem>

class DuplicateFinder {
public:
    struct FileInfo {
        std::string path;
        long long    size;
    };
    struct DuplicateGroup {
        long long                size;
        std::vector<std::string> paths;
    };

    DuplicateFinder() = default;

    // Scan a directory and report duplicate groups.
    bool findDuplicates(const std::string& dir);

    // Optionally remove duplicates (keep one copy).  Requires explicit
    // confirmation; the user picks WHICH copy to keep.
    bool removeDuplicatesInteractive(const std::string& dir);

    void showMenu();
private:
    // Read entire file content into a vector<char>.  Returns false on
    // read failure.
    bool readFileBytes(const std::string& path, std::vector<char>& out);
    // Byte-by-byte comparison of two files.  Returns true if identical.
    bool filesAreEqual(const std::string& a, const std::string& b);
};

#endif // DUPLICATE_FINDER_H
