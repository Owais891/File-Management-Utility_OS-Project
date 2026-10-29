// utils.cpp
// Implementation of the helper functions declared in utils.h.

#include "utils.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <filesystem>
#include <unistd.h>      // isatty
#include <termios.h>     // not strictly required but available
#include <sys/stat.h>

namespace fs = std::filesystem;

namespace utils {

// ---------------------------------------------------------------------
// ANSI color constants
// ---------------------------------------------------------------------
const char* RESET   = "\033[0m";
const char* BOLD    = "\033[1m";
const char* RED     = "\033[31m";
const char* GREEN   = "\033[32m";
const char* YELLOW   = "\033[33m";
const char* BLUE    = "\033[34m";
const char* CYAN    = "\033[36m";
const char* MAGENTA = "\033[35m";
const char* WHITE    = "\033[37m";

// ---------------------------------------------------------------------
// UI helpers
// ---------------------------------------------------------------------

void clearScreen() {
    // \033[2J clears the screen, \033[H moves the cursor to (0,0).
    std::cout << "\033[2J\033[H";
}

void pauseScreen() {
    // If stdin is already at EOF (e.g. piped input ran out), do not block.
    if (std::cin.eof()) {
        std::cout << "\n";
        return;
    }
    std::cout << YELLOW << "\nPress ENTER to return to the previous menu..."
              << RESET;
    // Clear any leftover input first so we don't read an old '\n'.
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string dummy;
    if (!std::getline(std::cin, dummy)) {
        // hit EOF — clear the failbit so subsequent reads don't crash
        std::cin.clear();
    }
}

void printHeader(const std::string& title) {
    std::string bar(60, '=');
    std::cout << CYAN << BOLD << "\n" << bar << "\n";
    // Center the title
    int spaces = (60 - static_cast<int>(title.size())) / 2;
    if (spaces < 0) spaces = 0;
    std::cout << std::string(spaces, ' ') << title << "\n"
              << bar << RESET << "\n\n";
}

void printSubHeader(const std::string& title) {
    std::cout << BLUE << BOLD << "\n--- " << title << " ---" << RESET << "\n";
}

void printSuccess(const std::string& msg) {
    std::cout << GREEN << "[SUCCESS] " << msg << RESET << "\n";
}

void printError(const std::string& msg) {
    std::cout << RED << "[ERROR]   " << msg << RESET << "\n";
}

void printWarning(const std::string& msg) {
    std::cout << YELLOW << "[WARNING] " << msg << RESET << "\n";
}

void printInfo(const std::string& msg) {
    std::cout << CYAN << "[INFO]    " << msg << RESET << "\n";
}

// ---------------------------------------------------------------------
// Input helpers
// ---------------------------------------------------------------------

std::string getInput(const std::string& prompt) {
    std::cout << GREEN << prompt << ": " << RESET;
    std::string line;
    if (!std::getline(std::cin, line)) {
        // EOF or read failure — return empty.  Leave eofbit set so the
        // caller (menu loop) can detect EOF and exit cleanly.
        return "";
    }
    // Trim leading/trailing whitespace
    auto first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    auto last = line.find_last_not_of(" \t\r\n");
    return line.substr(first, last - first + 1);
}

bool eofReached() {
    return std::cin.eof();
}

bool getInt(const std::string& prompt, int& out) {
    std::string line = getInput(prompt);
    if (line.empty()) return false;
    try {
        size_t idx = 0;
        int v = std::stoi(line, &idx);
        // Allow trailing whitespace only
        for (size_t i = idx; i < line.size(); ++i) {
            if (line[i] != ' ' && line[i] != '\t') return false;
        }
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

bool confirmAction(const std::string& message) {
    std::cout << YELLOW << BOLD << message << " (y/N): " << RESET;
    std::string answer;
    if (!std::getline(std::cin, answer)) {
        std::cin.clear();
        return false;
    }
    return (!answer.empty() && (answer[0] == 'y' || answer[0] == 'Y'));
}

// ---------------------------------------------------------------------
// Path / filename helpers
// ---------------------------------------------------------------------

bool isValidPath(const std::string& path) {
    if (path.empty()) return false;
    // Reject shell metacharacters that should never be in a real path
    // supplied by the user.  This is a defense-in-depth measure; we never
    // pass user input to system() so these are not security issues, but
    // we still want to refuse clearly bad input.
    static const std::string bad = ";|&`$()<>";
    for (char c : path) {
        if (bad.find(c) != std::string::npos) return false;
    }
    return true;
}

bool isHidden(const std::string& name) {
    // Linux convention: a name starting with '.' is hidden.
    // The special entries "." and ".." are also hidden but we usually
    // want to exclude them in listings.
    if (name.empty()) return false;
    if (name == "." || name == "..") return false;
    return name[0] == '.';
}

std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string getExtension(const std::string& path) {
    // Use std::filesystem::path::extension which returns the extension
    // including the leading dot, e.g. ".txt".  Strip the dot.
    fs::path p(path);
    std::string ext = p.extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
    return ext;
}

// ---------------------------------------------------------------------
// Size / permission formatting helpers
// ---------------------------------------------------------------------

std::string formatSize(long long bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double size = static_cast<double>(bytes);
    int unit = 0;
    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        ++unit;
    }
    std::ostringstream oss;
    oss.precision(2);
    oss << std::fixed << size << " " << units[unit];
    return oss.str();
}

std::string permissionToString(mode_t mode) {
    // Build "rwxrwxrwx" string from the lower 9 bits.
    std::string s;
    // Owner
    s += (mode & S_IRUSR) ? 'r' : '-';
    s += (mode & S_IWUSR) ? 'w' : '-';
    s += (mode & S_IXUSR) ? 'x' : '-';
    // Group
    s += (mode & S_IRGRP) ? 'r' : '-';
    s += (mode & S_IWGRP) ? 'w' : '-';
    s += (mode & S_IXGRP) ? 'x' : '-';
    // Others
    s += (mode & S_IROTH) ? 'r' : '-';
    s += (mode & S_IWOTH) ? 'w' : '-';
    s += (mode & S_IXOTH) ? 'x' : '-';
    return s;
}

std::string permissionToOctal(mode_t mode) {
    // Produce a 3-digit octal string for the user/group/other bits.
    // (We ignore setuid/setgid/sticky here for simplicity.)
    int o = (mode & S_IRWXU) >> 6;
    int g = (mode & S_IRWXG) >> 3;
    int w = (mode & S_IRWXO);
    std::ostringstream oss;
    oss << o << g << w;
    return oss.str();
}

} // namespace utils
