#include "agent.h"
#include "logger.h"
#include "classifier.h"
#include <windows.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace cybersentinel {

Agent::Agent(const std::string& config_file)
    : config_(std::make_unique<Config>(config_file)) {
}

Agent::~Agent() {
    stop();
}

bool Agent::initialize() {
    Logger::info("Initializing CyberSentinel DLP Agent...");

    // Load configuration
    if (!config_->load()) {
        Logger::error("Failed to load configuration");
        return false;
    }

    // Initialize system information
    initialize_system_info();

    // Initialize HTTP client
    http_client_ = std::make_unique<HttpClient>(
        config_->get_server_url()
    );

    // Register with server
    if (!register_agent()) {
        Logger::error("Failed to register agent with server");
        return false;
    }

    // Initialize monitors
    if (config_->is_file_monitoring_enabled()) {
        file_monitor_ = std::make_unique<FileMonitor>(
            config_->get_monitored_paths(),
            [this](const std::string& path, const std::string& event_type) {
                handle_file_event(path, event_type);
            }
        );

        if (!file_monitor_->start()) {
            Logger::error("Failed to start file monitor");
            return false;
        }
        Logger::info("File monitoring started");
    }

    if (config_->is_clipboard_monitoring_enabled()) {
        clipboard_monitor_ = std::make_unique<ClipboardMonitor>(
            [this](const std::string& content) {
                handle_clipboard_event(content);
            }
        );

        if (!clipboard_monitor_->start()) {
            Logger::error("Failed to start clipboard monitor");
            return false;
        }
        Logger::info("Clipboard monitoring started");
    }

    if (config_->is_usb_monitoring_enabled()) {
        usb_monitor_ = std::make_unique<USBMonitor>(
            [this](const std::string& device_name) {
                handle_usb_event(device_name);
            }
        );

        if (!usb_monitor_->start()) {
            Logger::error("Failed to start USB monitor");
            return false;
        }
        Logger::info("USB monitoring started");
    }

    initialized_ = true;
    Logger::info("Agent initialized successfully");
    return true;
}

void Agent::run() {
    if (!initialized_) {
        Logger::error("Agent not initialized");
        return;
    }

    running_ = true;
    Logger::info("Agent is now running...");

    // Start heartbeat thread
    std::thread heartbeat_thread([this]() {
        heartbeat_loop();
    });

    // Main loop
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Cleanup
    if (heartbeat_thread.joinable()) {
        heartbeat_thread.join();
    }

    Logger::info("Agent stopped");
}

void Agent::stop() {
    Logger::info("Stopping agent...");
    running_ = false;

    if (file_monitor_) {
        file_monitor_->stop();
    }
    if (clipboard_monitor_) {
        clipboard_monitor_->stop();
    }
    if (usb_monitor_) {
        usb_monitor_->stop();
    }
}

bool Agent::register_agent() {
    Logger::info("Registering agent with server...");

    // Build registration payload
    std::ostringstream payload;
    payload << "{"
            << "\"agent_id\":\"" << agent_id_ << "\","
            << "\"agent_name\":\"" << hostname_ << "\","
            << "\"hostname\":\"" << hostname_ << "\","
            << "\"os_type\":\"windows\","
            << "\"os_version\":\"" << os_version_ << "\","
            << "\"ip_address\":\"" << ip_address_ << "\","
            << "\"agent_version\":\"1.0.0\","
            << "\"capabilities\":{"
            << "\"file_monitoring\":" << (config_->is_file_monitoring_enabled() ? "true" : "false") << ","
            << "\"clipboard_monitoring\":" << (config_->is_clipboard_monitoring_enabled() ? "true" : "false") << ","
            << "\"usb_monitoring\":" << (config_->is_usb_monitoring_enabled() ? "true" : "false")
            << "}}";

    auto response = http_client_->post("/agents", payload.str());

    if (response.status_code == 200 || response.status_code == 201) {
        Logger::info("Agent registered successfully");
        return true;
    } else {
        Logger::error("Agent registration failed: HTTP " + std::to_string(response.status_code));
        return false;
    }
}

void Agent::send_heartbeat() {
    std::ostringstream payload;
    payload << "{"
            << "\"agent_id\":\"" << agent_id_ << "\","
            << "\"status\":\"online\"}";

    auto response = http_client_->put("/agents/" + agent_id_ + "/heartbeat", payload.str());

    if (response.status_code != 200) {
        Logger::warning("Heartbeat failed: HTTP " + std::to_string(response.status_code));
    }
}

void Agent::report_event(const std::string& event_type,
                        const std::string& severity,
                        const std::string& file_path,
                        const std::string& classification) {
    // Generate event ID
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    std::ostringstream event_id;
    event_id << "evt-" << agent_id_ << "-" << ms;

    // Build event payload
    std::ostringstream payload;
    payload << "{"
            << "\"event_id\":\"" << event_id.str() << "\","
            << "\"event_type\":\"" << event_type << "\","
            << "\"severity\":\"" << severity << "\","
            << "\"agent_id\":\"" << agent_id_ << "\","
            << "\"source_type\":\"endpoint\"";

    if (!file_path.empty()) {
        payload << ",\"file_path\":\"" << file_path << "\"";
    }

    if (!classification.empty()) {
        payload << ",\"classification\":" << classification;
    }

    payload << "}";

    auto response = http_client_->post("/events", payload.str());

    if (response.status_code == 200 || response.status_code == 201) {
        Logger::info("Event reported: " + event_type);
    } else {
        Logger::error("Failed to report event: HTTP " + std::to_string(response.status_code));
    }
}

void Agent::initialize_system_info() {
    // Get hostname
    char hostname[256];
    DWORD size = sizeof(hostname);
    if (GetComputerNameA(hostname, &size)) {
        hostname_ = std::string(hostname);
    } else {
        hostname_ = "UNKNOWN";
    }

    // Agent ID from config or generate
    agent_id_ = config_->get_agent_id();
    if (agent_id_ == "CHANGE_THIS_TO_UNIQUE_ID") {
        agent_id_ = "WIN-" + hostname_;
    }

    // OS version
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    #pragma warning(push)
    #pragma warning(disable: 4996) // Suppress deprecation warning
    if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
        std::ostringstream oss;
        oss << "Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
        os_version_ = oss.str();
    } else {
        os_version_ = "Windows (Unknown)";
    }
    #pragma warning(pop)

    // IP address (simplified - get first non-loopback)
    ip_address_ = "127.0.0.1"; // TODO: Implement proper IP detection

    Logger::info("System Info: " + hostname_ + " (" + os_version_ + ")");
}

void Agent::heartbeat_loop() {
    int interval = config_->get_heartbeat_interval();

    while (running_) {
        send_heartbeat();
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}

void Agent::handle_file_event(const std::string& file_path,
                               const std::string& event_type) {
    Logger::debug("File event: " + event_type + " - " + file_path);

    // Classify file content
    Classifier classifier;
    auto result = classifier.classify_file(file_path);

    if (!result.labels.empty()) {
        // Sensitive data detected
        std::ostringstream classification;
        classification << "{\"labels\":[";
        for (size_t i = 0; i < result.labels.size(); ++i) {
            if (i > 0) classification << ",";
            classification << "\"" << result.labels[i] << "\"";
        }
        classification << "],\"confidence\":" << result.confidence << "}";

        std::string severity = (result.confidence > 0.8) ? "critical" : "high";

        report_event("file_" + event_type, severity, file_path, classification.str());
    }
}

void Agent::handle_clipboard_event(const std::string& content) {
    Logger::debug("Clipboard event detected");

    // Classify clipboard content
    Classifier classifier;
    auto result = classifier.classify_text(content);

    if (!result.labels.empty()) {
        std::ostringstream classification;
        classification << "{\"labels\":[";
        for (size_t i = 0; i < result.labels.size(); ++i) {
            if (i > 0) classification << ",";
            classification << "\"" << result.labels[i] << "\"";
        }
        classification << "],\"confidence\":" << result.confidence << "}";

        report_event("clipboard_copy", "medium", "", classification.str());
    }
}

void Agent::handle_usb_event(const std::string& device_name) {
    Logger::info("USB device connected: " + device_name);
    report_event("usb_connected", "medium");
}

} // namespace cybersentinel
