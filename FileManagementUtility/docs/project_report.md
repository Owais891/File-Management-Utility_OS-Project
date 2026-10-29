# File Management Utility — Project Report

> Operating Systems Semester Project  
> BS Software Engineering  
> C++17 / Linux Ubuntu / POSIX

---

## 1. Title Page

**Project Title:** File Management Utility  
**Course:** Operating Systems  
**Language:** C++17  
**Platform:** Linux Ubuntu  
**Submission Format:** Source code + Makefile + Report  

---

## 2. Abstract

The *File Management Utility* is a menu-driven command-line application
developed in C++17 for Linux Ubuntu.  It provides a unified interface
for performing everyday file and directory operations — create, read,
append, copy, move, rename, delete — and for exploring Operating-System
concepts that are normally hidden behind high-level libraries and
graphical file managers.

The project demonstrates the use of the C++17 `<filesystem>` library for
portable file-system operations and the use of POSIX system calls —
`stat()`, `lstat()`, `chmod()`, `access()`, `getcwd()`, `chdir()`,
`getuid()`, `getgid()`, `readlink()` — for operations that the standard
library does not expose.  The application additionally implements a
recursive search engine, a directory tree viewer, an automatic file
organizer, a duplicate-file finder, and a persistent activity log.

The result is a beginner-friendly program suitable for a university
semester project presentation and viva: every OS concept that the
course covers is represented by a clearly identifiable source file and
menu option, with comments explaining which system call or library call
is being used and why.

---

## 3. Introduction

Modern desktop environments hide file-system complexity behind
graphical tools.  While convenient for end users, this hides the actual
Operating-System interfaces that a Software Engineering student must
understand to write system-level code.  The *File Management Utility*
was designed specifically to make these interfaces visible.

The application is built as a single binary that wires together several
small modules — each module corresponds to one or two course concepts —
into a single menu-driven console interface.  Every module is
self-contained and well-commented, so a student can read each one
independently to learn how a particular system call or library feature
works.

The project deliberately avoids:

* `system()` / `popen()` / shell commands — to avoid security risks and
  to ensure that every operation is performed by a library or system
  call that we control;
* external third-party libraries — to keep the project buildable with
  only `g++` and the standard library;
* database storage — to keep the persistence layer trivially simple
  (an append-only text log);
* running as root — to encourage safe permission-change behaviour.

---

## 4. Problem Statement

University students learning Operating Systems often struggle to bridge
the gap between the theoretical discussion of inodes, file permissions,
and system calls and the actual API that a programmer uses to access
these features.  Standard Linux tools such as `ls -l`, `chmod`, `find`,
and `stat` cover individual tasks but hide the underlying system calls
behind their own implementations.

The problem, therefore, is: *How can a single program expose every
major file-system concept of the OS course — file operations, directory
traversal, inode metadata, permissions, ownership, symbolic links,
and access control — through one coherent, beginner-friendly interface
that a student can read, compile, and explain during a viva?*

The **File Management Utility** is the proposed solution.

---

## 5. Objectives

1. Provide an integrated, menu-driven CLI for common file and directory
   operations on Linux.
2. Demonstrate the use of C++17 `std::filesystem` for portable
   file-system operations.
3. Demonstrate the use of POSIX system calls (`stat`, `lstat`, `chmod`,
   `access`, `getcwd`, `chdir`, `getuid`, `getgid`, `readlink`).
4. Show how Linux file permissions work in both symbolic and octal
   forms.
5. Implement safe recursive directory traversal that does not silently
   follow symbolic links into cycles.
6. Implement persistent activity logging.
7. Provide robust error handling at the OS boundary.
8. Keep the source code modular, commented, and understandable for
   beginners.

---

## 6. Scope of the Project

**In scope:**

* All file and directory operations listed in the objectives.
* Single-user, terminal-based interaction.
* POSIX-compliant Linux distributions (tested on Ubuntu).
* Activity logging in plain-text format.
* Educational explanation of OS concepts in code comments.

**Out of scope:**

* Networked or distributed file management.
* Multi-user authentication or access-control lists (ACLs).
* A graphical user interface.
* A database or indexed search engine.
* A version-controlled history of every byte change.

---

## 7. Functional Requirements

| ID    | Requirement |
|-------|-------------|
| FR-01 | The program shall present a welcome screen and a numbered main menu. |
| FR-02 | The program shall allow the user to create, read, append to, copy, move, rename, and delete files. |
| FR-03 | The program shall check whether a file exists, display its absolute path, and display its extension. |
| FR-04 | The program shall allow the user to create, list, rename, and remove (empty) directories. |
| FR-05 | The program shall display the current working directory and allow the user to change it. |
| FR-06 | The program shall display a directory tree up to a configurable depth. |
| FR-07 | The program shall count files, directories, and symbolic links in a directory tree. |
| FR-08 | The program shall list hidden files (dot-prefixed names) in a directory. |
| FR-09 | The program shall display directory statistics: total size, largest/smallest file, average size, hidden count, per-extension breakdown. |
| FR-10 | The program shall recursively search a directory for files by exact name, partial substring, or extension, case-insensitively. |
| FR-11 | The program shall display full metadata for a file: inode, type, size, owner, group, permissions, timestamps, symlink target. |
| FR-12 | The program shall display permissions in both symbolic and octal form. |
| FR-13 | The program shall allow the user to set permissions using either a 3-digit octal mode or an interactive symbolic menu. |
| FR-14 | The program shall confirm every destructive operation before performing it. |
| FR-15 | The program shall allow the user to organize files into category folders based on extension. |
| FR-16 | The program shall provide a dry-run preview before organizing files. |
| FR-17 | The program shall detect duplicate files by size + byte-by-byte comparison. |
| FR-18 | The program shall optionally remove duplicates, keeping one user-chosen copy. |
| FR-19 | The program shall append every significant operation to a persistent log file. |
| FR-20 | The program shall allow the user to view the latest N log entries. |
| FR-21 | The program shall return to the main menu after every operation. |
| FR-22 | The program shall never use `system()`, `popen()`, or shell commands. |
| FR-23 | The program shall not silently follow symbolic links during recursive scans. |

---

## 8. Non-Functional Requirements

| ID    | Requirement |
|-------|-------------|
| NFR-01 | The program shall compile with `g++ -std=c++17 -Wall -Wextra -O2` with **zero** errors and **zero** warnings. |
| NFR-02 | The program shall run on any modern Ubuntu system without additional libraries. |
| NFR-03 | The program shall be modular: each major feature shall live in its own `.h` / `.cpp` file. |
| NFR-04 | The source code shall be commented in beginner-friendly English. |
| NFR-05 | The program shall handle every error condition gracefully (no crash). |
| NFR-06 | The program shall not silently destroy user data. |
| NFR-07 | The program shall remain usable when ANSI colors are not supported. |
| NFR-08 | The program shall complete directory scans within a reasonable time for directories up to ~10 000 files. |
| NFR-09 | The program shall use POSIX system calls only when the standard library does not provide an equivalent. |
| NFR-10 | The build shall succeed with a single `make` command. |

---

## 9. Hardware and Software Requirements

### Hardware

| Component | Minimum        | Recommended   |
|-----------|----------------|---------------|
| CPU       | 1 GHz x86-64   | 2 GHz+ multi-core |
| RAM       | 1 GB           | 4 GB+         |
| Disk      | 200 MB free    | 1 GB free     |
| Display   | 80-column terminal | 120-column terminal |

### Software

| Component              | Version          |
|------------------------|------------------|
| Operating System       | Ubuntu 20.04 LTS+ (any modern Ubuntu) |
| Compiler               | `g++` 9.3+ (C++17) |
| Build tool             | GNU Make 4.x |
| C++ Standard Library   | libstdc++ (shipped with g++) |
| POSIX headers          | `<unistd.h>`, `<sys/stat.h>`, `<pwd.h>`, `<grp.h>` (provided by libc-dev) |

Install with:

```bash
sudo apt update
sudo apt install build-essential
```

---

## 10. System Architecture

### 10.1 High-level architecture

```
                       +-----------------+
                       |     main.cpp    |
                       |  (menu driver)  |
                       +--------+--------+
                                |
            +-------------------+-------------------+
            |        |          |          |        |
+-----+ +-----+ +-----+ +-----+ +-----+ +-----+ +-----+
|file | |dir  | |file | |meta | |perm | |org  | |dup  |
|mng  | |mng  | |srch | |data | |mod  | |aniz | |find |
+--+--+ +--+--+ +--+--+ +--+--+ +--+--+ +--+--+ +--+--+
   |       |       |       |       |       |       |
   +-------+-------+-------+-------+-------+-------+
                           |
                  +--------+--------+
                  |  activity_log   |
                  +--------+--------+
                           |
                  +--------+--------+
                  |     utils       |
                  +-----------------+
```

* **main.cpp** owns the menu loop and routes user choices to the
  appropriate module.
* Each module is a small class with its own `.h` / `.cpp` files and its
  own sub-menu.
* **activity_log** is a singleton shared across every module so that all
  operations land in one file.
* **utils** holds ANSI colors, input validation, and permission / size
  formatting helpers shared by every module.

### 10.2 Module responsibilities

| Module | Header        | Source         | Responsibility |
|--------|---------------|----------------|----------------|
| File Management     | `file_manager.h`     | `file_manager.cpp`     | CRUD on files |
| Directory Mgmt     | `directory_manager.h`| `directory_manager.cpp`| Dir ops + tree + stats |
| File Search        | `file_search.h`     | `file_search.cpp`      | Recursive search |
| Metadata           | `metadata.h`        | `metadata.cpp`         | `lstat`/`stat` metadata |
| Permissions        | `permissions.h`     | `permissions.cpp`      | `chmod` + UI |
| File Organizer     | `file_organizer.h`   | `file_organizer.cpp`  | Category-based organize |
| Duplicate Finder   | `duplicate_finder.h`| `duplicate_finder.cpp` | Byte-level dedup |
| Activity Log       | `activity_log.h`    | `activity_log.cpp`     | Persistent log |
| Utils              | `utils.h`           | `utils.cpp`            | Shared helpers |
| Main               | —                   | `main.cpp`             | Menu driver |

---

## 11. Module Descriptions

### 11.1 File Management (`file_manager.cpp`)

Provides 10 operations on regular files: create, read, append, copy,
move, rename, delete, existence check, absolute-path display, and
extension display.  All operations use C++17 `std::filesystem` and
`<fstream>`.  The reader refuses to silently overwrite a destination
without confirmation.

### 11.2 Directory Management (`directory_manager.cpp`)

Provides 10 directory operations including a tree printer that does
**not** follow symlinks.  Uses POSIX `getcwd()` and `chdir()` for
the CWD-related operations.  Recursive traversal uses
`fs::recursive_directory_iterator` with
`directory_options::skip_permission_denied`.

### 11.3 File Search (`file_search.cpp`)

Walks a directory recursively and matches files by exact name, partial
substring, or extension.  Case-insensitive matching is implemented via
`std::tolower`.

### 11.4 Metadata (`metadata.cpp`)

The flagship OS-concept module.  Calls `lstat()` first to inspect the
path itself (so symlinks are detected as links, not their targets).
Calls `stat()` only when the path is a symlink, in order to also
report the target.  Uses `access()` for the current user's
read/write/execute rights, and `getpwuid()` / `getgrgid()` to look up
owner and group names.

### 11.5 Permissions (`permissions.cpp`)

Demonstrates `chmod()`.  Provides four ways to interact with
permissions: view, set via octal mode, set via an interactive symbolic
menu, and check current-user access via `access()`.  Includes a built-in
demonstration that creates a temporary file and cycles through several
common permission modes (644, 600, 755, 000).

### 11.6 File Organizer (`file_organizer.cpp`)

Builds a categorization plan in memory first, prints a dry-run
preview, and only then asks for confirmation.  Refuses to organize a
destination that lives inside the source (would cause infinite
recursion).  Renames instead of overwriting when a target filename
already exists.

### 11.7 Duplicate Finder (`duplicate_finder.cpp`)

Two-stage algorithm:

1. Group all regular files by `st_size`.
2. For each group, do a byte-by-byte comparison using 8 KB buffers.

Reports per-group file count and total storage recoverable.  Optional
interactive removal lets the user pick which copy to keep.

### 11.8 Activity Log (`activity_log.cpp`)

A singleton that appends a structured line per operation.  Creates the
`logs/` directory automatically on first use.  Provides a "view latest
N" feature.

### 11.9 Utils (`utils.cpp`)

Holds ANSI color codes, input helpers (`getInput`, `getInt`,
`confirmAction`), path sanity checks (`isValidPath`,
`isHidden`), case-insensitive string helper (`toLower`), extension
extraction, size formatting, and permission-bit-to-string helpers
(`permissionToString`, `permissionToOctal`).

---

## 12. Algorithms for Major Features

### 12.1 Recursive file search

```
function walk(directory D, predicate P, results R):
    iterator = recursive_directory_iterator(D, skip_permission_denied)
    for each entry E in iterator:
        if E is a regular file AND P(E):
            append {path, name, size} to R
    return R
```

### 12.2 Directory tree (no symlink following)

```
function printTree(dir D, prefix, depth, maxDepth):
    if depth >= maxDepth: return
    entries = sort(directory_iterator(D))
    for each (entry E, index i):
        isLast = (i == size - 1)
        print prefix + (isLast ? "└── " : "├── ") + name(E)
        if E is directory AND not symlink:
            newPrefix = prefix + (isLast ? "    " : "│   ")
            printTree(E, newPrefix, depth+1, maxDepth)
```

### 12.3 Duplicate file detection

```
function findDuplicates(directory D):
    all = collect all regular files (path, size)
    group by size:
        groups[size] = list of files of that size
    duplicates = []
    for each (size, files) in groups:
        if files.size < 2: continue
        visited = [false] * files.size
        for i in 0..files.size-1:
            if visited[i]: continue
            group = [files[i].path]
            visited[i] = true
            for j in i+1..files.size-1:
                if not visited[j] AND byteCompare(files[i].path, files[j].path):
                    group.append(files[j].path)
                    visited[j] = true
            if group.size >= 2:
                duplicates.append(group)
    return duplicates

function byteCompare(a, b):
    if size(a) != size(b): return false
    read a and b in 8 KB chunks; compare each chunk via memcmp
    return true iff all chunks match
```

### 12.4 File organization with dry-run preview

```
function buildPlan(src, dst):
    absSrc = canonical(absolute(src))
    absDst = canonical(absolute(dst))
    if absDst is inside absSrc: reject  # prevents recursion
    plan = []
    for each regular file F in recursive_walk(src):
        if F is inside absDst: continue
        ext = extension(F)
        category = categoryFor(ext)
        destDir = absDst / category
        destPath = destDir / filename(F)
        plan.append({source: F, destinationDir, destinationPath, ext, category})
    return plan

function organize(src, dst):
    if not previewAndConfirm(buildPlan(src, dst)): return false
    plan = buildPlan(src, dst)   # rebuild
    create_directories(dst)
    for each move M in plan:
        create_directories(M.destinationDir)
        if exists(M.destinationPath):
            pick a non-clashing alternative name with "(N)" suffix
        rename(M.source, M.destinationPath) or copy+remove
        log success or failure
    print summary
```

### 12.5 Permission change with safety

```
function setOctalMode(path, octalStr):
    if not validOctal(octalStr): reject
    oldMode = lstat(path).st_mode
    show old vs new side by side
    if not confirm(): cancel
    if path is symlink: warn user that chmod will modify the TARGET
    if chmod(path, newMode) != 0:
        print strerror(errno)
        hint: may need sudo for this single operation
    log success
```

---

## 13. Flowchart Descriptions

### 13.1 Main program flow

```
START
  |
  v
showWelcome()
  |
  v
+--------------------+
| Show main menu     |<-----------+
+--------------------+            |
  |                               |
  v                               |
Read user choice                  |
  |                               |
  +-- 1 -> File Mgmt submenu ---->+
  +-- 2 -> Directory Mgmt submenu >+
  +-- 3 -> Search submenu -------->+
  +-- 4 -> Metadata submenu ------>+
  +-- 5 -> Permissions submenu --->+
  +-- 6 -> Dir Size Analyzer ------>+
  +-- 7 -> File Organizer submenu >+
  +-- 8 -> Duplicate Finder submenu>+
  +-- 9 -> Hidden Files submenu --->+
  +-- 10 -> Activity Log ---------->+
  +-- 11 -> Help ------------------>+
  +-- 0 -> Exit -------------------+
  |                               |
  v                               |
END                              |
```

### 13.2 Generic operation flow

```
Read user input (path/s)
  |
  v
Validate path (isValidPath)
  |
  +-- invalid -> [ERROR], return to submenu
  |
  v
Check existence / type
  |
  +-- not found / wrong type -> [ERROR], log, return
  |
  v
Perform operation (catch error_code)
  |
  +-- failure -> [ERROR] + strerror(errno), log FAILURE
  |
  v
Log SUCCESS
  |
  v
Print [SUCCESS] message
  |
  v
Return to submenu
```

### 13.3 Destructive operation flow (with confirmation)

```
Read user input
  |
  v
Validate + check existence
  |
  v
Show what will be changed
  |
  v
Ask "y/N" confirmation
  |
  +-- N -> [INFO] cancelled, log CANCELLED, return
  |
  v
Perform destructive operation
  |
  v
Log result
  |
  v
Return to submenu
```

---

## 14. Testing Strategy

The testing strategy is manual, scenario-based, and centered on the
project's test files in `test_files/`.  For each test:

1. Note the test case ID.
2. Set up the inputs.
3. Predict the expected result.
4. Run the program and observe the actual result.
5. Mark Pass / Fail.

Tests are grouped by module.  Each module has positive tests (valid
inputs) and negative tests (invalid inputs, permission-denied,
missing files, etc.).

The full test plan is provided in **Section 15** below.

> The "Actual Result" and "Pass/Fail" columns are intentionally left
> blank for the student to complete during real testing.  This report
> does **not** claim that any test has passed.

---

## 15. Test Cases with Expected Results

### 15.1 File Management

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TF-01 | Choose 1 -> 1; enter `./test_files/new1.txt` | `[SUCCESS] Created empty file` | | |
| TF-02 | Choose 1 -> 1; enter an existing path | `[ERROR] File already exists` | | |
| TF-03 | Choose 1 -> 2; enter `./test_files/sample.txt` | File contents displayed | | |
| TF-04 | Choose 1 -> 2; enter a non-existent path | `[ERROR] File does not exist` | | |
| TF-05 | Choose 1 -> 2; enter a directory path | `[ERROR] Path points to a directory` | | |
| TF-06 | Choose 1 -> 3; enter `./test_files/new1.txt`; text "abc" | `[SUCCESS] Text appended` | | |
| TF-07 | Choose 1 -> 4; src `./test_files/sample.txt`; dst `./copy.txt` | `[SUCCESS] Copied` | | |
| TF-08 | Choose 1 -> 4; dst already exists | `Overwrite? (y/N)` prompt | | |
| TF-09 | Choose 1 -> 5; src `./copy.txt`; dst `./moved.txt` | `[SUCCESS] Moved` | | |
| TF-10 | Choose 1 -> 6; old `./moved.txt`; new `./renamed.txt` | `[SUCCESS] Renamed` | | |
| TF-11 | Choose 1 -> 7; path `./renamed.txt`; answer `y` | `[SUCCESS] Deleted file` | | |
| TF-12 | Choose 1 -> 7; answer `N` | `[INFO] Deletion cancelled` | | |
| TF-13 | Choose 1 -> 8; enter `./test_files/sample.txt` | `[SUCCESS] Path exists` | | |
| TF-14 | Choose 1 -> 8; enter `/nonexistent/x` | `[WARNING] Path does not exist` | | |
| TF-15 | Choose 1 -> 9; enter `./test_files/sample.txt` | `Absolute path: /home/...` | | |
| TF-16 | Choose 1 -> 10; enter `./test_files/sample.txt` | `Extension: .txt` | | |
| TF-17 | Choose 1 -> 10; enter a path with no extension | `No extension` message | | |

### 15.2 Directory Management

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TD-01 | Choose 2 -> 1; enter `./newdir` | `[SUCCESS] Created directory` | | |
| TD-02 | Choose 2 -> 1; enter an existing path | `[ERROR] Path already exists` | | |
| TD-03 | Choose 2 -> 2; enter `./test_files` | Table of entries | | |
| TD-04 | Choose 2 -> 2; enter a non-directory path | `[ERROR] Path is not a directory` | | |
| TD-05 | Choose 2 -> 3; rename `./newdir` to `./newdir2` | `[SUCCESS] Renamed directory` | | |
| TD-06 | Choose 2 -> 4; remove empty `./newdir2`; answer `y` | `[SUCCESS] Removed` | | |
| TD-07 | Choose 2 -> 4; remove non-empty directory | `[ERROR] Directory is not empty` | | |
| TD-08 | Choose 2 -> 5 | Prints `Current working directory: ...` | | |
| TD-09 | Choose 2 -> 6; navigate to `./test_files` | `[SUCCESS] Changed to: ...` | | |
| TD-10 | Choose 2 -> 7; tree of `./test_files` | Tree printed | | |
| TD-11 | Choose 2 -> 8; count `./test_files` | Counts shown | | |
| TD-12 | Choose 2 -> 9; hidden list of `.` | Hidden entries listed (or "none") | | |
| TD-13 | Choose 2 -> 10; stats of `./test_files` | Full statistics block | | |

### 15.3 File Search

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TS-01 | 3 -> 1; dir `./test_files`; name `sample.txt` | 1 match | | |
| TS-02 | 3 -> 1; name `SAMPLE.TXT` (case-insensitive) | 1 match | | |
| TS-03 | 3 -> 2; partial `samp` | 1 match | | |
| TS-04 | 3 -> 3; ext `txt` | All `.txt` files | | |
| TS-05 | 3 -> 3; ext `.pdf` (with dot) | Same as `pdf` | | |
| TS-06 | 3 -> 1; non-existent name | `No matches found` | | |
| TS-07 | 3 -> 1; invalid directory | `[ERROR] Invalid or non-directory path` | | |

### 15.4 Metadata

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TM-01 | 4 -> 1; path `./test_files/sample.txt` | Full metadata block | | |
| TM-02 | 4 -> 1; path `./test_files` (directory) | Metadata of directory | | |
| TM-03 | 4 -> 1; symlink (create one with `ln -s`) | "Symbolic link: yes" + target block | | |
| TM-04 | 4 -> 1; non-existent | `[ERROR] lstat() failed` | | |

### 15.5 Permissions

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TP-01 | 5 -> 1; path `./test_files/sample.txt` | Permissions shown | | |
| TP-02 | 5 -> 2; path `./test_files/sample.txt`; mode `600`; answer `y` | `[SUCCESS] Permissions changed` | | |
| TP-03 | 5 -> 2; mode `999` (invalid digit) | `[ERROR] Invalid octal mode` | | |
| TP-04 | 5 -> 2; mode `64` (only 2 digits) | `[ERROR] Invalid octal mode` | | |
| TP-05 | 5 -> 2; mode `644`; answer `N` | `[INFO] Permission change cancelled` | | |
| TP-06 | 5 -> 3; interactive menu | Permissions set per user choices | | |
| TP-07 | 5 -> 4; path `./test_files/sample.txt` | Access matrix shown | | |
| TP-08 | 5 -> 5; path | Current permissions shown | | |
| TP-09 | 5 -> 6; run demo | Demo cycles through 644 -> 600 -> 755 -> 000 -> 644 | | |
| TP-10 | 5 -> 2; chmod on a file owned by root | `[ERROR] chmod() failed: Operation not permitted` + hint | | |

### 15.6 Directory Size Analyzer

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TZ-01 | 6; path `./test_files` | Stats block with files, sizes, etc. | | |
| TZ-02 | 6; path `./nonexistent` | `[ERROR] Invalid or non-directory path` | | |

### 15.7 File Organizer

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TO-01 | 7 -> 1; src `./test_files`; dst `./OrganizedFiles` | Dry-run plan printed | | |
| TO-02 | 7 -> 2; src `./test_files`; dst `./OrganizedFiles`; answer `y` | Files moved into category folders | | |
| TO-03 | 7 -> 2; src = dst | `[ERROR] Source and destination are the same` | | |
| TO-04 | 7 -> 2; dst inside src | `[ERROR] Destination is inside source` | | |
| TO-05 | 7 -> 2; src contains a `.pdf` | PDF ends up in Documents/ | | |

### 15.8 Duplicate Finder

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TDF-01 | 8 -> 1; dir `./test_files` (no duplicates) | `No duplicate files found` | | |
| TDF-02 | Make two byte-identical copies; 8 -> 1 | 1 duplicate group reported | | |
| TDF-03 | 8 -> 2; interactive removal | User chooses which to keep; others deleted | | |
| TDF-04 | 8 -> 1; dir with two files of same size but different content | No duplicates reported | | |

### 15.9 Hidden Files

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TH-01 | 9 -> 1; dir `.` | Hidden entries (`.bashrc` etc.) listed | | |
| TH-02 | 9 -> 1; dir `./test_files` (no hidden files) | `(no hidden entries)` | | |

### 15.10 Activity Log

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TL-01 | 10; entries `5` | Latest 5 log entries shown | | |
| TL-02 | After any operation, check `logs/activity.log` | New entry appended | | |

### 15.11 Error Cases / Edge Cases

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TE-01 | Empty input at main menu | `[ERROR] Invalid input` | | |
| TE-02 | Menu choice `99` | `[ERROR] Invalid choice` | | |
| TE-03 | Path with `;` (shell metachar) | `[ERROR] Invalid path` | | |
| TE-04 | Copy to a directory the user can't write | `[ERROR] Copy failed: Permission denied` | | |
| TE-05 | Tree on `/proc` (loops / perms) | Limited output, no crash | | |

### 15.12 Symbolic Links

| Test ID | Setup / Input | Expected Result | Actual | Pass/Fail |
|---------|---------------|-----------------|--------|-----------|
| TSL-01 | Create symlink, then 4 -> 1 | `Symbolic link: yes` + target block | | |
| TSL-02 | Tree print containing a symlink | `[symlink — not followed]` printed | | |
| TSL-03 | 5 -> 2 chmod on a symlink | `[WARNING] Path is a symbolic link. chmod() will modify the TARGET` | | |

---

## 16. Operating System Concepts Used

| Concept | API / Header | Module |
|---------|--------------|--------|
| File-system operations | `std::filesystem::copy_file`, `rename`, `remove`, `create_directories` | file_manager, directory_manager, file_organizer |
| Directory traversal | `std::filesystem::recursive_directory_iterator` | directory_manager, file_search, duplicate_finder |
| Inode & metadata | `lstat()`, `stat()`, `st_ino`, `st_size`, `st_mtime`, `st_uid`, `st_gid` | metadata |
| Linux file permissions | `chmod()`, `access()`, `S_IRUSR`...`S_IXOTH` | permissions, utils |
| User and group ownership | `getuid()`, `getgid()`, `getpwuid()`, `getgrgid()` | metadata, permissions |
| System calls | `lstat`, `stat`, `chmod`, `access`, `getcwd`, `chdir`, `readlink` | across modules |
| Error handling at OS interface | `errno`, `strerror`, `std::error_code` | across modules |
| File-system hierarchy | `absolute()`, `weakly_canonical()`, `directory_iterator` | utils, file_search, file_organizer |
| Symbolic links | `S_ISLNK`, `readlink`, recursive-iterator skip | metadata, directory_manager |
| File access control | `access(R_OK|W_OK|X_OK)` | metadata, permissions |

---

## 17. Advantages

1. **Single binary, no dependencies** — only `g++` and the standard
   library are required.
2. **Beginner-friendly source code** — each module is self-contained,
   heavily commented, and ~150–300 lines long.
3. **OS-concepts visible** — every POSIX call used is named in the
   output and in the source comments.
4. **Safe** — no `system()` or `popen()`; no shell injection vector.
5. **Persistent log** — every operation is auditable.
6. **Robust** — error paths are handled, no crashes on bad input.
7. **Portable across POSIX** — runs on any Linux distribution with
   C++17 `g++`.
8. **Demonstration-ready** — built-in demo mode for permissions.

---

## 18. Limitations

1. The text-file viewer is designed for plain text.  Binary files
   are detected and warned but still printed.
2. The duplicate finder uses byte-by-byte comparison (forbidden from
   using external hashing libraries).  For very large directories a
   middle filter (e.g. an internal rolling hash) would help.
3. `chmod()` on a symlink modifies the target by POSIX definition.
   The program warns the user but cannot avoid this behaviour.
4. Logical file sizes are reported; actual on-disk block usage is
   not.
5. The directory-tree printer is capped at depth 5 by default.
6. There is no GUI; all interaction is via the terminal.

---

## 19. Future Enhancements

1. Add an internal hash function (e.g. a 64-bit rolling checksum) as
   a middle filter in the duplicate finder.
2. Add `inotify(7)`-based "watch" mode.
3. Add ACL management (`getfacl` / `setfacl`) as an advanced
   permission feature.
4. Add an interactive TUI (ncurses) for arrow-key navigation.
5. Add internationalization (i18n) of the menu text.
6. Add a configuration file for default paths and category mapping.
7. Add a basic SHA-1 / SHA-256 implementation in pure C++ for
   stronger duplicate detection.
8. Add a "diff" feature to compare two files visually.

---

## 20. Conclusion

The **File Management Utility** successfully demonstrates every major
file-system concept of the Operating Systems course through a single,
integrated, beginner-friendly C++17 program.  The student who builds,
runs, and reads this project will see — in concrete, runnable code —
how `lstat()`, `stat()`, `chmod()`, `access()`, `getcwd()`, `chdir()`,
`getuid()`, `getgid()`, and `readlink()` are used in real programs,
how `std::filesystem` complements them, and how a careful engineer
handles errors at the OS boundary without crashing or losing user
data.

The modular design means that any single concept can be taught by
reading one source file in isolation, and the menu-driven interface
makes the same concept easy to demonstrate live during a viva.

---

## 21. References

1. IEEE Std 1003.1-2017 (POSIX.1-2017) — `stat(2)`, `lstat(2)`,
   `chmod(2)`, `access(2)`, `getcwd(3)`, `chdir(2)`,
   `readlink(2)`, `getpwuid(3)`, `getgrgid(3)`.
2. ISO/IEC 14882:2017 (C++17 Standard) — `<filesystem>` library
   clauses.
3. `man 2 stat`, `man 2 chmod`, `man 2 access`, `man 2 getcwd`,
   `man 2 readlink` — Ubuntu Linux man pages.
4. B. Stroustrup, *The C++ Programming Language*, 4th ed.,
   Addison-Wesley.
5. W. Richard Stevens, *Advanced Programming in the UNIX Environment*,
   3rd ed., Addison-Wesley.
6. A. Silberschatz, P. Galvin, G. Gagne, *Operating System Concepts*,
   10th ed., Wiley.
7. cppreference.com — `std::filesystem` reference.
8. Ubuntu Manual Pages — `man 7 inode`, `man 5 dir`.
