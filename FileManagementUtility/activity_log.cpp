#include "activity_log.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

ActivityLog& ActivityLog::getInstance() {
    static ActivityLog instance;
    return instance;
}

ActivityLog::ActivityLog() {
    // Prefer "logs/activity.log" relative to CWD (matches project layout).
    logFilePath_ = "logs/activity.log";
}

bool ActivityLog::ensureLogDir() {
    std::error_code ec;
    fs::create_directories("logs", ec);
    return !ec;
}

std::string ActivityLog::currentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char buf[32] = {0};
    if (lt) {
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
    }
    return std::string(buf);
}

void ActivityLog::log(const std::string& operation,
                      const std::string& source,
                      const std::string& destination,
                      const std::string& result,
                      const std::string& error) {
    if (!ensureLogDir()) {
        utils::printWarning("Could not create logs/ directory; entry not saved.");
        return;
    }
    std::ofstream ofs(logFilePath_, std::ios::app);
    if (!ofs) {
        utils::printWarning("Could not open activity log file: " + logFilePath_);
        return;
    }
    ofs << "[" << currentTimestamp() << "] "
        << "OP="   << operation  << " | "
        << "SRC="  << source     << " | "
        << "DST="  << destination<< " | "
        << "RES="  << result     << " | "
        << "ERR="  << error
        << "\n";
    ofs.close();
}

void ActivityLog::viewLatest(int entries) {
    std::ifstream ifs(logFilePath_);
    if (!ifs) {
        utils::printWarning("No activity log found yet at: " + logFilePath_);
        return;
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(ifs, line)) lines.push_back(line);
    ifs.close();

    if (lines.empty()) {
        utils::printInfo("Activity log is empty.");
        return;
    }
    int start = static_cast<int>(lines.size()) - entries;
    if (start < 0) start = 0;
    utils::printSubHeader("Activity Log (latest " +
                          std::to_string(lines.size() - start) + " entries)");
    for (int i = start; i < static_cast<int>(lines.size()); ++i) {
        std::cout << lines[i] << "\n";
    }
    std::cout << "\nTotal log entries: " << lines.size() << "\n";
}
