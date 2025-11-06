#ifndef CYBERSENTINEL_AGENT_H
#define CYBERSENTINEL_AGENT_H

#include <string>
#include <memory>
#include <atomic>
#include "config.h"
#include "file_monitor.h"
#include "clipboard_monitor.h"
#include "usb_monitor.h"
#include "http_client.h"

namespace cybersentinel {

class Agent {
public:
    explicit Agent(const std::string& config_file);
    ~Agent();

    // Delete copy constructor and assignment
    Agent(const Agent&) = delete;
    Agent& operator=(const Agent&) = delete;

    // Main operations
    bool initialize();
    void run();
    void stop();

    // Agent registration
    bool register_agent();
    void send_heartbeat();

    // Event reporting
    void report_event(const std::string& event_type,
                     const std::string& severity,
                     const std::string& file_path = "",
                     const std::string& classification = "");

private:
    // Configuration
    std::unique_ptr<Config> config_;

    // Monitors
    std::unique_ptr<FileMonitor> file_monitor_;
    std::unique_ptr<ClipboardMonitor> clipboard_monitor_;
    std::unique_ptr<USBMonitor> usb_monitor_;

    // HTTP client for server communication
    std::unique_ptr<HttpClient> http_client_;

    // Control flags
    std::atomic<bool> running_{false};
    std::atomic<bool> initialized_{false};

    // Agent info
    std::string agent_id_;
    std::string hostname_;
    std::string os_version_;
    std::string ip_address_;

    // Helper methods
    void initialize_system_info();
    void heartbeat_loop();
    void handle_file_event(const std::string& file_path,
                           const std::string& event_type);
    void handle_clipboard_event(const std::string& content);
    void handle_usb_event(const std::string& device_name);
};

} // namespace cybersentinel

#endif // CYBERSENTINEL_AGENT_H
