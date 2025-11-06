#ifndef CYBERSENTINEL_USB_MONITOR_H
#define CYBERSENTINEL_USB_MONITOR_H

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <windows.h>

namespace cybersentinel {

using USBEventCallback = std::function<void(const std::string& device_name)>;

class USBMonitor {
public:
    explicit USBMonitor(USBEventCallback callback);
    ~USBMonitor();

    bool start();
    void stop();

private:
    USBEventCallback callback_;
    std::atomic<bool> running_{false};
    std::thread monitor_thread_;
    HWND hwnd_{nullptr};

    void monitor_loop();
    void check_usb_devices();
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

} // namespace cybersentinel

#endif // CYBERSENTINEL_USB_MONITOR_H
