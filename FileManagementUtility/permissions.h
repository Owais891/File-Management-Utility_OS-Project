// permissions.h
// Linux file-permissions module.
//
// Operating System Concepts Demonstrated:
//   - POSIX chmod() to change file permissions.
//   - Symbolic <-> octal conversion of permission bits.
//   - access() for the current user's effective permissions.
//   - getuid() / getgid() for the current process identity.

#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <string>

class Permissions {
public:
    Permissions() = default;

    // Show current permissions of a path (symbolic + octal).
    bool viewPermissions(const std::string& path);

    // Set permissions using an octal mode string e.g. "754".
    bool setOctalMode(const std::string& path, const std::string& octalStr);

    // Set permissions interactively by toggling r/w/x for u/g/o.
    bool setSymbolicMenu(const std::string& path);

    // Check read/write/execute access for the current user.
    bool checkAccess(const std::string& path);

    // Show before/after comparison.
    bool comparePermissions(const std::string& path);

    // Run a built-in demonstration on a test file.
    bool runDemonstration();

    void showMenu();
private:
    // Helper: validate "ddd" octal string and convert to mode_t.
    bool parseOctal(const std::string& s, mode_t& out);
};

#endif // PERMISSIONS_H
