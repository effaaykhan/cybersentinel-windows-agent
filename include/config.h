#ifndef CYBERSENTINEL_CONFIG_H
#define CYBERSENTINEL_CONFIG_H

#include <string>
#include <vector>

namespace cybersentinel {

class Config {
public:
    explicit Config(const std::string& config_file);
    ~Config() = default;

    bool load();

    // Getters
    std::string get_server_url() const { return server_url_; }
    std::string get_agent_id() const { return agent_id_; }
    std::string get_agent_name() const { return agent_name_; }
    int get_heartbeat_interval() const { return heartbeat_interval_; }

    bool is_file_monitoring_enabled() const { return file_monitoring_enabled_; }
    bool is_clipboard_monitoring_enabled() const { return clipboard_monitoring_enabled_; }
    bool is_usb_monitoring_enabled() const { return usb_monitoring_enabled_; }

    std::vector<std::string> get_monitored_paths() const { return monitored_paths_; }

private:
    std::string config_file_;

    // Configuration values
    std::string server_url_;
    std::string agent_id_;
    std::string agent_name_;
    int heartbeat_interval_;

    bool file_monitoring_enabled_;
    bool clipboard_monitoring_enabled_;
    bool usb_monitoring_enabled_;

    std::vector<std::string> monitored_paths_;
};

} // namespace cybersentinel

#endif // CYBERSENTINEL_CONFIG_H
