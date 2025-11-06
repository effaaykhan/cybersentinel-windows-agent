#include "file_monitor.h"
#include "logger.h"
#include <sstream>

namespace cybersentinel {

FileMonitor::FileMonitor(const std::vector<std::string>& paths, FileEventCallback callback)
    : monitored_paths_(paths), callback_(callback) {
}

FileMonitor::~FileMonitor() {
    stop();
}

bool FileMonitor::start() {
    if (running_) {
        Logger::warning("File monitor already running");
        return true;
    }

    running_ = true;

    for (const auto& path : monitored_paths_) {
        // Expand environment variables
        char expanded_path[MAX_PATH];
        ExpandEnvironmentStringsA(path.c_str(), expanded_path, MAX_PATH);

        // Start monitoring thread for each path
        monitor_threads_.emplace_back([this, p = std::string(expanded_path)]() {
            monitor_directory(p);
        });

        Logger::info("Started monitoring: " + std::string(expanded_path));
    }

    return true;
}

void FileMonitor::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    // Close directory handles to unblock ReadDirectoryChangesW
    for (auto handle : dir_handles_) {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }
    dir_handles_.clear();

    // Wait for threads to finish
    for (auto& thread : monitor_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    monitor_threads_.clear();

    Logger::info("File monitor stopped");
}

void FileMonitor::monitor_directory(const std::string& path) {
    // Open directory handle
    HANDLE dir_handle = CreateFileA(
        path.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr
    );

    if (dir_handle == INVALID_HANDLE_VALUE) {
        Logger::error("Failed to open directory: " + path);
        return;
    }

    dir_handles_.push_back(dir_handle);

    const DWORD buffer_size = 4096;
    char buffer[buffer_size];
    DWORD bytes_returned;

    OVERLAPPED overlapped = {0};
    overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    while (running_) {
        // Read directory changes
        BOOL result = ReadDirectoryChangesW(
            dir_handle,
            buffer,
            buffer_size,
            TRUE, // Watch subdirectories
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytes_returned,
            &overlapped,
            nullptr
        );

        if (!result) {
            Logger::error("ReadDirectoryChangesW failed for: " + path);
            break;
        }

        // Wait for event
        DWORD wait_result = WaitForSingleObject(overlapped.hEvent, 1000);

        if (wait_result == WAIT_TIMEOUT) {
            continue;
        }

        if (wait_result != WAIT_OBJECT_0) {
            break;
        }

        if (!GetOverlappedResult(dir_handle, &overlapped, &bytes_returned, FALSE)) {
            if (GetLastError() != ERROR_OPERATION_ABORTED) {
                Logger::error("GetOverlappedResult failed");
            }
            break;
        }

        // Reset event
        ResetEvent(overlapped.hEvent);

        // Process notifications
        FILE_NOTIFY_INFORMATION* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

        while (true) {
            // Convert filename from wide string
            std::wstring wfilename(fni->FileName, fni->FileNameLength / sizeof(WCHAR));
            std::string filename(wfilename.begin(), wfilename.end());

            std::string full_path = path + "\\" + filename;
            std::string event_type = get_event_type(fni->Action);

            // Call callback
            if (callback_) {
                callback_(full_path, event_type);
            }

            // Check if there are more entries
            if (fni->NextEntryOffset == 0) {
                break;
            }

            fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<BYTE*>(fni) + fni->NextEntryOffset
            );
        }
    }

    CloseHandle(overlapped.hEvent);
    CloseHandle(dir_handle);
}

std::string FileMonitor::get_event_type(DWORD action) {
    switch (action) {
        case FILE_ACTION_ADDED:
            return "created";
        case FILE_ACTION_REMOVED:
            return "deleted";
        case FILE_ACTION_MODIFIED:
            return "modified";
        case FILE_ACTION_RENAMED_OLD_NAME:
        case FILE_ACTION_RENAMED_NEW_NAME:
            return "moved";
        default:
            return "unknown";
    }
}

} // namespace cybersentinel
