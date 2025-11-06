#ifndef CYBERSENTINEL_FILE_MONITOR_H
#define CYBERSENTINEL_FILE_MONITOR_H

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <windows.h>

namespace cybersentinel {

using FileEventCallback = std::function<void(const std::string& file_path, const std::string& event_type)>;

class FileMonitor {
public:
    explicit FileMonitor(const std::vector<std::string>& paths, FileEventCallback callback);
    ~FileMonitor();

    bool start();
    void stop();

private:
    std::vector<std::string> monitored_paths_;
    FileEventCallback callback_;
    std::atomic<bool> running_{false};
    std::vector<std::thread> monitor_threads_;
    std::vector<HANDLE> dir_handles_;

    void monitor_directory(const std::string& path);
    std::string get_event_type(DWORD action);
};

} // namespace cybersentinel

#endif // CYBERSENTINEL_FILE_MONITOR_H
