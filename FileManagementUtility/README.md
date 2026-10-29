# File Management Utility

An Operating Systems semester project demonstrating C++17 `std::filesystem`
and Linux/POSIX system calls for managing files and directories on Ubuntu.

---

## 1. Project Title

**File Management Utility** — a menu-driven command-line application written
in C++17 for Linux Ubuntu that helps users manage files and directories,
search for files, inspect metadata, understand Linux file permissions,
and organize files efficiently.

---

## 2. Introduction

The **File Management Utility** is a beginner-friendly C++17 console
application built specifically for the Operating Systems course.  It
provides a single integrated environment for everyday file-management
tasks while making the underlying operating-system concepts visible
and easy to explain during a viva.

The program is organised into small, well-commented modules so that each
major Operating-System concept — file-system operations, directory
traversal, inode metadata, Linux file permissions, user/group
ownership, POSIX system calls, symbolic links, and file access control —
has a clearly identifiable place in the source code.

Every operation is logged to `logs/activity.log`, every destructive
action requires explicit confirmation, and the application never uses
`system()`, `popen()`, or any shell command — all file operations are
done through C++17 `<filesystem>` and POSIX system calls.

---

## 3. Problem Statement

Modern desktop environments hide file-system complexity behind graphical
tools.  While convenient, this hides the actual Operating-System
interfaces that a Software Engineering student must understand: `stat()`,
`lstat()`, `chmod()`, `access()`, `getcwd()`, `chdir()`, `getuid()`,
`getgid()`, inode fields, permission bits, and symbolic links.

We needed a single, easy-to-demonstrate program that exposes these
interfaces through a clean menu, so that a student can:

* perform real file operations,
* inspect inode metadata,
* change and observe permissions,
* search directories recursively,
* organize files by extension,
* detect duplicate files,
* keep a persistent activity log,

all without leaving the terminal and without typing shell commands.

---

## 4. Project Objectives

1. Provide a single, integrated, menu-driven CLI for common file and
   directory operations on Linux.
2. Demonstrate the use of C++17 `std::filesystem` for portable
   file-system operations.
3. Demonstrate the use of POSIX system calls (`stat`, `lstat`, `chmod`,
   `access`, `getcwd`, `chdir`, `getuid`, `getgid`, `readlink`).
4. Show how Linux file permissions work — both symbolic and octal forms.
5. Implement safe recursive directory traversal that does not silently
   follow symbolic links into cycles.
6. Implement persistent activity logging.
7. Provide robust error handling at the OS boundary.
8. Keep the source code modular, commented, and understandable for
   beginners.

---

## 5. Main Features

| #  | Module                       | Capabilities |
|----|------------------------------|--------------|
| 1  | File Management              | Create / read / append / copy / move / rename / delete files; check existence; absolute path; extension display |
| 2  | Directory Management         | Create / list / rename / remove (empty) / show CWD / navigate / display tree / count entries / find hidden / calculate statistics |
| 3  | File Search                  | Recursive search by exact name, partial substring, or extension; case-insensitive |
| 4  | File Metadata                | Inode, type, size, owner uid/gid, mtime, permissions (symbolic + octal), readability, writability, executability, hidden flag, symlink flag, link target |
| 5  | Linux File Permissions       | View, set via octal mode, set via interactive symbolic menu, check access, compare before/after, demo on a test file |
| 6  | Directory Size Analyzer      | File count, dir count, link count, total bytes, largest/smallest file, average size, hidden count, per-extension breakdown |
| 7  | Automatic File Organizer     | Categorize files into Documents/Images/Videos/Audio/Archives/Code/Others; dry-run preview; rename instead of overwrite |
| 8  | Duplicate File Finder        | Size-grouping then byte-by-byte comparison; per-group reporting; interactive removal with user-chosen keep copy |
| 9  | Hidden Files & Directories   | List dot-prefixed entries with type, size, permissions; explanation that hidden ≠ secure |
| 10 | Activity Logging             | Append-only log at `logs/activity.log` with timestamp, operation, source, destination, result, error |

---

## 6. Technologies Used

* **Programming Language:** C++17
* **Compiler:** `g++`
* **Target OS:** Linux Ubuntu
* **Standard Library:** `<filesystem>`, `<fstream>`, `<vector>`, `<map>`,
  `<set>`, `<functional>`, `<ctime>`, `<iomanip>`
* **POSIX Headers Used:** `<unistd.h>`, `<sys/stat.h>`, `<pwd.h>`,
  `<grp.h>`
* **External libraries:** None
* **Build system:** GNU Make

---

## 7. System Requirements

### Hardware
* Any PC that runs Ubuntu Desktop or Server.
* Minimum: 1 GHz CPU, 1 GB RAM, 200 MB free disk.

### Software
* Ubuntu 20.04 LTS or newer (any modern Ubuntu works).
* `g++` supporting C++17 (`g++ --version` should report 7.1 or newer;
  Ubuntu 20.04 ships `g++` 9.3 by default).
* `make` (GNU Make).
* A terminal that supports ANSI escape codes for color output.
  Colors are optional — if a terminal does not support them, the
  escape sequences are simply ignored.

---

## 8. Project Folder Structure

```
FileManagementUtility/
├── main.cpp                # Menu driver
├── file_manager.{h,cpp}    # File operations
├── directory_manager.{h,cpp}  # Directory operations
├── file_search.{h,cpp}     # Recursive search
├── metadata.{h,cpp}        # stat/lstat metadata
├── permissions.{h,cpp}     # chmod + permission UI
├── file_organizer.{h,cpp}  # Auto-organize
├── duplicate_finder.{h,cpp}# Duplicate detection
├── activity_log.{h,cpp}    # Persistent logging
├── utils.{h,cpp}           # Shared helpers
├── Makefile                # Build system
├── README.md               # This file
├── logs/
│   └── activity.log         # Auto-created on first operation
├── test_files/
│   ├── sample.txt
│   ├── document.pdf
│   └── image.jpg
└── docs/
    └── project_report.md
```

---

## 9. Installation Instructions

The project ships as plain source code — there is no installer.

1. Copy the `FileManagementUtility/` folder to your home directory or
   any working folder.
2. Open a terminal and `cd` into the project root:

   ```bash
   cd ~/FileManagementUtility
   ```
3. (Optional) Verify the compiler is installed:

   ```bash
   g++ --version    # should print 7.1 or newer
   make --version
   ```

If `g++` is not installed:

```bash
sudo apt update
sudo apt install build-essential
```

---

## 10. Compilation Instructions

### Using `make` (recommended)

```bash
cd ~/FileManagementUtility
make
```

This compiles every `.cpp` into a `.o` and links them into an executable
named `file_manager`.  The full compiler invocation used by `make` is:

```
g++ -std=c++17 -Wall -Wextra -O2 -o file_manager \
    main.cpp file_manager.cpp directory_manager.cpp file_search.cpp \
    metadata.cpp permissions.cpp file_organizer.cpp duplicate_finder.cpp \
    activity_log.cpp utils.cpp
```

### Manual g++ (without Make)

If `make` is unavailable, compile directly:

```bash
g++ -std=c++17 -Wall -Wextra -O2 -o file_manager \
    main.cpp file_manager.cpp directory_manager.cpp file_search.cpp \
    metadata.cpp permissions.cpp file_organizer.cpp duplicate_finder.cpp \
    activity_log.cpp utils.cpp
```

---

## 11. Execution Instructions

```bash
./file_manager
```

You will see a welcome screen, then the main menu.  Press `1`–`11` to
access modules, `0` to exit.  Each submenu returns to the main menu
after every operation.  Every destructive operation requires
`y`/`Y` confirmation.

---

## 12. Usage Examples

### Example 1 — Create a text file

```
Main menu -> 1 (File Management) -> 1 (Create a new text file)
Enter new file path: ./test_files/hello.txt
[SUCCESS] Created empty file: ./test_files/hello.txt
```

### Example 2 — Append to a file

```
Main menu -> 1 -> 3 (Append text to a file)
Enter file path to append to: ./test_files/hello.txt
Enter text to append (single line): Hello, OS class!
[SUCCESS] Text appended successfully.
```

### Example 3 — Read a file

```
Main menu -> 1 -> 2
Enter file path to read: ./test_files/hello.txt
--- Contents of ./test_files/hello.txt ---
Hello, OS class!
```

### Example 4 — Search by extension

```
Main menu -> 3 -> 3 (Search by extension)
Enter starting directory: ./test_files
Enter extension (e.g. pdf): txt
--- Search results (1 match(es)) ---
  #    Size          Path
  ---  -----------   --------------------------------
  1    14 B          ./test_files/hello.txt
```

### Example 5 — View file metadata

```
Main menu -> 4 -> 1
Enter path: ./test_files/hello.txt
  Name             : hello.txt
  Absolute path    : /home/<user>/FileManagementUtility/test_files/hello.txt
  Extension        : .txt
  File type        : Regular file
  Size (bytes)     : 14
  Size (human)     : 14 B
  Last modified    : 2026-10-28 06:30:11
  Owner (uid)      : <username> (1001)
  Group (gid)      : <group> (1001)
  Permissions sym  : rw-r--r--
  Permissions oct  : 644
  Readable         : yes
  Writable         : yes
  Executable       : no
  Hidden file      : no
  Symbolic link    : no
```

### Example 6 — Change permissions (octal)

```
Main menu -> 5 -> 2
Enter path: ./test_files/hello.txt
Enter octal mode (e.g. 754): 600
  Current : rw-r--r-- (644)
  New     : rw------- (600)
Apply this change? (y/N): y
[SUCCESS] Permissions changed.
```

### Example 7 — Directory size analyzer

```
Main menu -> 6
Enter directory path: ./test_files
  Regular files        : 3
  Directories          : 0
  Symbolic links       : 0
  Hidden entries       : 0
  Total bytes (regular): 12345  (12.06 KB)
  Largest file         : ./test_files/image.jpg (10.00 KB)
  ...
```

### Example 8 — Organize files

```
Main menu -> 7 -> 2 (Organize now)
Enter source directory: ./test_files
Enter destination directory: ./OrganizedFiles
[ dry-run preview printed ]
Proceed with these moves? (y/N): y
[SUCCESS] Moved ./test_files/hello.txt -> ./OrganizedFiles/Documents/hello.txt
...
  Success : 3
  Failed  : 0
```

### Example 9 — Find duplicates

```
Main menu -> 8 -> 1
Enter directory to scan: ./test_files
Scanned directory : ./test_files
Regular files      : 3
Duplicate groups   : 0
No duplicate files found.
```

### Example 10 — View activity log

```
Main menu -> 10
How many latest entries to show (default 20): 5
--- Activity Log (latest 5 entries) ---
[2026-10-28 06:30:11] OP=CREATE_FILE | SRC=./test_files/hello.txt | DST= | RES=SUCCESS | ERR=
[2026-10-28 06:30:25] OP=APPEND_FILE | SRC=./test_files/hello.txt | DST= | RES=SUCCESS | ERR=
...
```

---

## 13. Example Outputs

The above examples illustrate the typical terminal output for each
module.  Sample outputs are also visible in the screenshots folder if
you capture them during your viva.  The exact byte counts and
timestamps in the outputs depend on the contents of `test_files/` and
the time you run the program.

---

## 14. Error Handling Details

The program handles the following error conditions gracefully (no
crashes, no silent data loss):

| Error                              | How it is handled |
|------------------------------------|---------------------|
| Invalid menu choice                | Re-prompts the user; the menu re-renders |
| Empty input                        | Treated as invalid choice, re-prompt |
| Invalid path (contains shell metacharacters) | `utils::isValidPath` rejects it |
| Missing file or directory          | Checks existence before any operation |
| Permission denied                 | Catches `std::error_code`; prints `[ERROR] chmod() failed: ...` |
| Destination already exists         | Asks the user for confirmation before overwriting |
| Recursive traversal of inaccessible subdirectory | `directory_options::skip_permission_denied` skips it silently |
| Symlink cycles during tree print  | Tree printer does NOT recurse into symlinks |
| Trying to move dest into itself   | Detected by prefix comparison and rejected |
| Invalid octal permission string    | `parseOctal()` rejects non-3-digit or non-octal input |
| `lstat` / `chmod` failure         | `strerror(errno)` is printed |
| Logging failure                   | Warning printed, program continues |

Important safety guarantees:

* The application **never** calls `system()`, `popen()`, `exec*`, or
  forks a shell.  All file operations use `std::filesystem` and POSIX
  calls.
* The application **never** follows symbolic links during recursive
  traversal.  The `metadata` module uses `lstat()` so a symlink is
  reported as a link, not as its target.
* The application **never** overwrites a file silently — every
  overwrite requires explicit `y` confirmation.

---

## 15. Operating System Concepts Demonstrated

| Concept                                | Where it appears |
|----------------------------------------|------------------|
| File-system operations                 | `file_manager.cpp`, `file_organizer.cpp` |
| Directory traversal                    | `directory_manager.cpp`, `file_search.cpp`, `duplicate_finder.cpp` |
| Inode / metadata                       | `metadata.cpp` (`lstat`, `stat`, `st_ino`, `st_size`, `st_mtime`) |
| Linux file permissions                 | `permissions.cpp`, `utils.cpp` (`permissionToString`, `permissionToOctal`) |
| User and group ownership               | `metadata.cpp` (`getpwuid`, `getgrgid`), `permissions.cpp` (`getuid`, `getgid`) |
| POSIX system calls                     | `lstat`, `stat`, `chmod`, `access`, `getcwd`, `chdir`, `readlink` |
| Error handling at the OS interface      | Every module uses `std::error_code` and `errno`/`strerror` |
| File-system hierarchy                  | `absolute()`, `weakly_canonical()`, `directory_iterator` |
| Symbolic links                         | `metadata.cpp` (link detection), `directory_manager.cpp` (tree refuses to follow) |
| File access control                    | `permissions.cpp`, `access()` calls |

C++17 `std::filesystem` is used for portable file and directory
operations; POSIX system calls are used wherever the standard library
does not provide an equivalent (e.g. `chmod`, `access`, `getuid`,
`getcwd`, `chdir`, `readlink`).

---

## 16. Known Limitations

1. The text-file reader is meant for plain-text files.  Binary files
   (PDF, JPG, ZIP) are detected and warned about, but their bytes are
   still printed to the terminal (which may look corrupt).
2. The duplicate-file finder uses **byte-by-byte comparison** after
   grouping by size.  This is O(n·m) per group but is intentionally
   simple — the project specification forbids external hashing
   libraries.  For very large directories, an internal hash function
   (e.g. a basic 64-bit checksum) could be added as a middle filter.
3. The directory tree printer is capped at depth 5 by default to
   prevent accidental huge outputs.
4. The directory-size analyzer reports **logical** file sizes.  The
   actual disk space consumed (block allocation) may differ and is not
   reported.
5. `chmod()` on a symbolic link follows the link by POSIX definition
   and modifies the *target's* permissions.  The program warns the
   user about this.
6. Permission changes on files you do not own require sudo for that
   single chmod operation.  The program prints a hint when `chmod`
   fails with `EPERM`.  Do **not** run the whole application as root.
7. ANSI colors may not render on extremely old terminals.  They are
   ignored gracefully.

---

## 17. Future Enhancements

1. Add an internal rolling-hash filter between size-grouping and
   byte-compare to scale the duplicate finder.
2. Add a configuration file (e.g. `~/.fmuconfig`) for default source /
   destination paths.
3. Add more categories (Fonts, DataSets, Books) to the file organizer.
4. Add inode-count reporting per directory.
5. Add a "watch" mode that re-runs scans when a directory changes
   (`inotify(7)`).
6. Add a basic SHA-1 / SHA-256 implementation in pure C++ for stronger
   duplicate detection (the standard library provides none).
7. Add internationalization (i18n) of the menu text.
8. Add a TUI library (e.g. ncurses) for arrow-key navigation.
9. Add support for setting ACLs (`setfacl`) as an advanced permission
   feature.
10. Add batch operations (operate on multiple files at once).

---

## 18. License

This project is provided for academic use by the original author.
You may modify and extend it for your own learning purposes.
