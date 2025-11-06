#include "usb_monitor.h"
#include "logger.h"
#include <dbt.h>

namespace cybersentinel {

USBMonitor::USBMonitor(USBEventCallback callback)
    : callback_(callback) {
}

USBMonitor::~USBMonitor() {
    stop();
}

bool USBMonitor::start() {
    if (running_) {
        Logger::warning("USB monitor already running");
        return true;
    }

    running_ = true;
    monitor_thread_ = std::thread([this]() { monitor_loop(); });

    Logger::info("USB monitoring started");
    return true;
}

void USBMonitor::stop() {
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

    Logger::info("USB monitor stopped");
}

void USBMonitor::monitor_loop() {
    // Create message-only window
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "USBMonitorClass";

    if (!RegisterClassEx(&wc)) {
        Logger::error("Failed to register USB monitor window class");
        return;
    }

    hwnd_ = CreateWindowEx(
        0,
        "USBMonitorClass",
        nullptr,
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!hwnd_) {
        Logger::error("Failed to create USB monitor message window");
        return;
    }

    // Register for device notifications
    DEV_BROADCAST_DEVICEINTERFACE notification_filter = {0};
    notification_filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notification_filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    HDEVNOTIFY h_notify = RegisterDeviceNotification(
        hwnd_,
        &notification_filter,
        DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES
    );

    if (!h_notify) {
        Logger::error("Failed to register for device notifications");
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
    UnregisterDeviceNotification(h_notify);
    DestroyWindow(hwnd_);
    UnregisterClass("USBMonitorClass", GetModuleHandle(nullptr));
}

void USBMonitor::check_usb_devices() {
    // This would require WMI or other APIs to enumerate USB devices
    // For now, we rely on WM_DEVICECHANGE messages
}

LRESULT CALLBACK USBMonitor::window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == WM_CREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return 0;
    }

    USBMonitor* monitor = reinterpret_cast<USBMonitor*>(
        GetWindowLongPtr(hwnd, GWLP_USERDATA)
    );

    if (msg == WM_DEVICECHANGE && monitor) {
        if (wparam == DBT_DEVICEARRIVAL) {
            DEV_BROADCAST_HDR* hdr = reinterpret_cast<DEV_BROADCAST_HDR*>(lparam);

            if (hdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                DEV_BROADCAST_VOLUME* vol = reinterpret_cast<DEV_BROADCAST_VOLUME*>(lparam);

                // Get drive letter
                char drive_letter = 'A';
                DWORD mask = vol->dbcv_unitmask;
                while (mask > 1) {
                    mask >>= 1;
                    drive_letter++;
                }

                std::string device_name = std::string(1, drive_letter) + ":\\";

                if (monitor->callback_) {
                    monitor->callback_(device_name);
                }
            }
        }
        return TRUE;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

} // namespace cybersentinel
