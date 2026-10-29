// metadata.h
// File-metadata inspector.
//
// Operating System Concepts Demonstrated:
//   - POSIX lstat() to inspect a path WITHOUT following symlinks.
//   - POSIX stat() to inspect the TARGET of a symlink.
//   - Inode fields: st_size, st_uid, st_gid, st_mode, st_mtime.
//   - Distinguishing a symlink from its target.
//   - access() to check read/write/execute permission for the CURRENT user.

#ifndef METADATA_H
#define METADATA_H

#include <string>

class Metadata {
public:
    Metadata() = default;
    // Show complete metadata for a path.  Uses lstat() so symlinks
    // themselves are inspected (not their targets).
    bool showMetadata(const std::string& path);

    void showMenu();
};

#endif // METADATA_H
