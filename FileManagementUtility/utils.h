// utils.h
// Common helper functions used across the File Management Utility.
// Provides ANSI color codes, input validation, confirmation prompts,
// size formatting, permission-string formatting, and small UI helpers.
//
// Operating System Concepts Demonstrated:
//   - Use of POSIX <sys/stat.h> mode_t to interpret permission bits.
//   - Helper layer that wraps user input so the rest of the program
//     can stay clean and beginner-friendly.

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sys/types.h>   // mode_t

namespace utils {

// ---------------------------------------------------------------------
// ANSI terminal color codes.  When the terminal does not support ANSI
// colors the escape sequences are simply ignored (displayed as nothing),
// so the application remains usable in plain terminals.
// ---------------------------------------------------------------------
extern const char* RESET;
extern const char* BOLD;
extern const char* RED;
extern const char* GREEN;
extern const char* YELLOW;
extern const char* BLUE;
extern const char* CYAN;
extern const char* MAGENTA;
extern const char* WHITE;

// ---------------------------------------------------------------------
// UI helper functions
// ---------------------------------------------------------------------
void clearScreen();
void pauseScreen();
void printHeader(const std::string& title);     // Big title banner
void printSubHeader(const std::string& title);  // Smaller section header
void printSuccess(const std::string& msg);
void printError(const std::string& msg);
void printWarning(const std::string& msg);
void printInfo(const std::string& msg);

// ---------------------------------------------------------------------
// Input helpers
// ---------------------------------------------------------------------
// Read a non-empty trimmed line from stdin.
std::string getInput(const std::string& prompt);
// Read a non-negative integer; returns false on parse failure.
bool getInt(const std::string& prompt, int& out);
// Yes/No confirmation prompt.  Returns true only on 'y' / 'Y'.
bool confirmAction(const std::string& message);
// Returns true once stdin has reached EOF (end-of-file).  Used by menu
// loops so the program exits cleanly when input is piped in.
bool eofReached();

// ---------------------------------------------------------------------
// Path / filename helpers
// ---------------------------------------------------------------------
// Basic sanity check: rejects empty paths and paths containing shell
// metacharacters that could be misused.  Does NOT execute anything.
bool isValidPath(const std::string& path);
// Returns true if the file/dir name starts with '.' (Linux hidden).
bool isHidden(const std::string& name);
// Convert a string to lower-case (used for case-insensitive search).
std::string toLower(const std::string& s);
// Extract the file extension (without the leading dot).  Returns "" if
// the file has no extension.
std::string getExtension(const std::string& path);

// ---------------------------------------------------------------------
// Size / permission formatting helpers
// ---------------------------------------------------------------------
// Convert a byte count into a human-readable string (B/KB/MB/GB).
std::string formatSize(long long bytes);
// Convert a mode_t permission value to symbolic form e.g. "rwxr-xr--".
std::string permissionToString(mode_t mode);
// Convert a mode_t permission value to a 3-digit octal string e.g. "754".
std::string permissionToOctal(mode_t mode);

} // namespace utils

#endif // UTILS_H
