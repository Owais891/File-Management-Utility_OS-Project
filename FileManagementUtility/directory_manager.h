// directory_manager.h
// Directory-level operations: create / list / rename / remove (empty) /
// current working directory / navigate / display tree / count / hidden /
// directory statistics.
//
// Operating System Concepts Demonstrated:
//   - Directory traversal using C++17 std::filesystem recursive iterators.
//   - Use of POSIX getcwd() / chdir() to read and change the CWD.
//   - Avoidance of symbolic-link cycles: we use non-recursive iterators
//     for the tree feature with explicit symlink handling.

#ifndef DIRECTORY_MANAGER_H
#define DIRECTORY_MANAGER_H

#include <string>

class DirectoryManager {
public:
    DirectoryManager() = default;

    bool createDirectory(const std::string& path);
    bool listDirectory(const std::string& path);
    bool renameDirectory(const std::string& oldPath, const std::string& newPath);
    bool removeEmptyDirectory(const std::string& path);
    bool displayCurrentDirectory();
    bool navigateDirectory(const std::string& path);
    bool displayTree(const std::string& path, unsigned maxDepth = 5);
    bool countEntries(const std::string& path);
    bool findHidden(const std::string& path);
    bool calculateStats(const std::string& path);

    void showMenu();
};

#endif // DIRECTORY_MANAGER_H
