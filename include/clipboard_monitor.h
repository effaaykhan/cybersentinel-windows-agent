#ifndef CYBERSENTINEL_CLIPBOARD_MONITOR_H
#define CYBERSENTINEL_CLIPBOARD_MONITOR_H

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <windows.h>

namespace cybersentinel {

using ClipboardEventCallback = std::function<void(const std::string& content)>;

class ClipboardMonitor {
public:
    explicit ClipboardMonitor(ClipboardEventCallback callback);
    ~ClipboardMonitor();

    bool start();
    void stop();

private:
    ClipboardEventCallback callback_;
    std::atomic<bool> running_{false};
    std::thread monitor_thread_;
    HWND hwnd_{nullptr};

    void monitor_loop();
    std::string get_clipboard_text();
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

} // namespace cybersentinel

#endif // CYBERSENTINEL_CLIPBOARD_MONITOR_H
