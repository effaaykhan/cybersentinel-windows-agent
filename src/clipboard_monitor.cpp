#include "clipboard_monitor.h"
#include "logger.h"

namespace cybersentinel {

ClipboardMonitor::ClipboardMonitor(ClipboardEventCallback callback)
    : callback_(callback) {
}

ClipboardMonitor::~ClipboardMonitor() {
    stop();
}

bool ClipboardMonitor::start() {
    if (running_) {
        Logger::warning("Clipboard monitor already running");
        return true;
    }

    running_ = true;
    monitor_thread_ = std::thread([this]() { monitor_loop(); });

    Logger::info("Clipboard monitoring started");
    return true;
}

void ClipboardMonitor::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    // Post quit message to window
    if (hwnd_) {
        PostMessage(hwnd_, WM_QUIT, 0, 0);
    }

    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }

    Logger::info("Clipboard monitor stopped");
}

void ClipboardMonitor::monitor_loop() {
    // Create message-only window
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "ClipboardMonitorClass";

    if (!RegisterClassEx(&wc)) {
        Logger::error("Failed to register window class");
        return;
    }

    hwnd_ = CreateWindowEx(
        0,
        "ClipboardMonitorClass",
        nullptr,
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!hwnd_) {
        Logger::error("Failed to create message window");
        return;
    }

    // Add to clipboard chain
    if (!AddClipboardFormatListener(hwnd_)) {
        Logger::error("Failed to add clipboard listener");
        DestroyWindow(hwnd_);
        return;
    }

    // Message loop
    MSG msg;
    while (running_ && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    RemoveClipboardFormatListener(hwnd_);
    DestroyWindow(hwnd_);
    UnregisterClass("ClipboardMonitorClass", GetModuleHandle(nullptr));
}

std::string ClipboardMonitor::get_clipboard_text() {
    if (!OpenClipboard(nullptr)) {
        return "";
    }

    std::string result;
    HANDLE h_data = GetClipboardData(CF_TEXT);

    if (h_data) {
        char* text = static_cast<char*>(GlobalLock(h_data));
        if (text) {
            result = text;
            GlobalUnlock(h_data);
        }
    }

    CloseClipboard();
    return result;
}

LRESULT CALLBACK ClipboardMonitor::window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == WM_CREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return 0;
    }

    ClipboardMonitor* monitor = reinterpret_cast<ClipboardMonitor*>(
        GetWindowLongPtr(hwnd, GWLP_USERDATA)
    );

    if (msg == WM_CLIPBOARDUPDATE && monitor) {
        std::string content = monitor->get_clipboard_text();
        if (!content.empty() && monitor->callback_) {
            monitor->callback_(content);
        }
        return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

} // namespace cybersentinel
