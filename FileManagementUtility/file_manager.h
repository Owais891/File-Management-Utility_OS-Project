// file_manager.h
// File-level operations: create / read / append / copy / move / rename /
// delete / existence / absolute path / extension display.
//
// Operating System Concepts Demonstrated:
//   - File-system hierarchy navigation.
//   - Use of C++17 <filesystem> for portable file operations.
//   - Use of <fstream> for byte-level file I/O.
//   - Avoidance of system() / popen() — we use library calls only.

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

class FileManager {
public:
    FileManager() = default;

    // 1. Create a new (empty) text file.
    bool createFile(const std::string& path);

    // 2. Read and display a text file's contents.
    bool readFile(const std::string& path);

    // 3. Append user-supplied text to an existing file.
    bool appendToFile(const std::string& path);

    // 4. Copy a file.  Refuses to overwrite the destination silently.
    bool copyFile(const std::string& src, const std::string& dst);

    // 5. Move a file.  Refuses to overwrite the destination silently.
    bool moveFile(const std::string& src, const std::string& dst);

    // 6. Rename a file in place.  Asks for confirmation if a target with
    // the new name already exists.
    bool renameFile(const std::string& oldPath, const std::string& newPath);

    // 7. Delete a file after user confirmation.
    bool deleteFile(const std::string& path);

    // 8. Check whether a file exists.
    bool fileExists(const std::string& path);

    // 9. Display the absolute path of a file.
    bool displayAbsolutePath(const std::string& path);

    // 10. Display the file extension.
    bool displayExtension(const std::string& path);

    // Interactive sub-menu shown from main.cpp.
    void showMenu();
};

#endif // FILE_MANAGER_H
