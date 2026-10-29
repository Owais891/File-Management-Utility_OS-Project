// activity_log.h
// Persistent activity logger.  Records every significant operation
// performed by the user to logs/activity.log inside the project folder.
//
// Operating System Concepts Demonstrated:
//   - Persistent logging to a real file on disk.
//   - Use of <ctime> / localtime for human-readable timestamps.
//   - Use of std::filesystem::create_directories to ensure the logs
//     directory exists before writing.

#ifndef ACTIVITY_LOG_H
#define ACTIVITY_LOG_H

#include <string>

class ActivityLog {
public:
    // Singleton access — every module shares the same logger so that all
    // operations land in a single consistent log file.
    static ActivityLog& getInstance();

    // Append a structured entry to the log file.
    //   operation : e.g. "CREATE_FILE", "COPY_FILE", "CHMOD"
    //   source    : path of the file/dir being operated on (may be empty)
    //   destination: secondary path (e.g. copy destination, may be empty)
    //   result    : "SUCCESS" or "FAILURE"
    //   error     : error description if any (may be empty)
    void log(const std::string& operation,
             const std::string& source    = "",
             const std::string& destination = "",
             const std::string& result    = "",
             const std::string& error     = "");

    // Show the last N entries from the log file on the terminal.
    void viewLatest(int entries = 20);

private:
    ActivityLog();
    ActivityLog(const ActivityLog&)            = delete;
    ActivityLog& operator=(const ActivityLog&) = delete;

    // Make sure logs/ exists.  Returns true on success.
    bool ensureLogDir();

    // Build a timestamp string in the form "YYYY-MM-DD HH:MM:SS".
    std::string currentTimestamp();

    std::string logFilePath_;
};

#endif // ACTIVITY_LOG_H
