#include "config.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace cybersentinel {

Config::Config(const std::string& config_file)
    : config_file_(config_file),
      heartbeat_interval_(60),
      file_monitoring_enabled_(true),
      clipboard_monitoring_enabled_(true),
      usb_monitoring_enabled_(true) {
}

bool Config::load() {
    try {
        std::ifstream file(config_file_);
        if (!file.is_open()) {
            Logger::error("Could not open config file: " + config_file_);
            return false;
        }

        json config = json::parse(file);

        // Server configuration
        if (config.contains("server_url")) {
            server_url_ = config["server_url"].get<std::string>();
        } else {
            Logger::error("Missing server_url in configuration");
            return false;
        }

        // Agent configuration
        if (config.contains("agent_id")) {
            agent_id_ = config["agent_id"].get<std::string>();
        } else {
            agent_id_ = "CHANGE_THIS_TO_UNIQUE_ID";
        }

        if (config.contains("agent_name")) {
            agent_name_ = config["agent_name"].get<std::string>();
        } else {
            agent_name_ = "CyberSentinel Agent";
        }

        if (config.contains("heartbeat_interval")) {
            heartbeat_interval_ = config["heartbeat_interval"].get<int>();
        }

        // Monitoring configuration
        if (config.contains("monitoring")) {
            auto monitoring = config["monitoring"];

            if (monitoring.contains("file_system")) {
                file_monitoring_enabled_ = monitoring["file_system"].get<bool>();
            }

            if (monitoring.contains("clipboard")) {
                clipboard_monitoring_enabled_ = monitoring["clipboard"].get<bool>();
            }

            if (monitoring.contains("usb_devices")) {
                usb_monitoring_enabled_ = monitoring["usb_devices"].get<bool>();
            }

            if (monitoring.contains("monitored_paths")) {
                monitored_paths_ = monitoring["monitored_paths"].get<std::vector<std::string>>();
            }
        }

        Logger::info("Configuration loaded successfully");
        Logger::info("Server URL: " + server_url_);
        Logger::info("Agent ID: " + agent_id_);

        return true;

    } catch (const std::exception& e) {
        Logger::error("Failed to parse config file: " + std::string(e.what()));
        return false;
    }
}

} // namespace cybersentinel
