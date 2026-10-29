// file_organizer.h
// Automatic file organizer.
//
// Operating System Concepts Demonstrated:
//   - Recursive directory scanning.
//   - File move operations via C++17 std::filesystem::rename.
//   - Categorization by file extension.

#ifndef FILE_ORGANIZER_H
#define FILE_ORGANIZER_H

#include <string>
#include <vector>
#include <filesystem>

class FileOrganizer {
public:
    struct MovePlan {
        std::string source;
        std::string destinationDir;
        std::string destinationPath;
        std::string extension;
        std::string category;
    };

    FileOrganizer();

    // Preview the planned organization (dry-run) and return whether the
    // user confirmed.
    bool previewAndConfirm(const std::string& src, const std::string& dst);

    // Actually perform the organization.
    bool organize(const std::string& src, const std::string& dst);

    void showMenu();
private:
    // Build the plan for organizing all regular files in `src`.
    //   dst must exist or be creatable.
    std::vector<MovePlan> buildPlan(const std::string& src,
                                     const std::string& dst);

    // Pick a category label for a given extension.
    std::string categoryFor(const std::string& ext);

    // Ensure the destination subfolder exists, returning its absolute path.
    std::string ensureDestSubfolder(const std::string& dst,
                                     const std::string& category);
};

#endif // FILE_ORGANIZER_H
