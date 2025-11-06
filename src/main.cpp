#include "agent.h"
#include "logger.h"
#include <windows.h>
#include <iostream>
#include <csignal>

using namespace cybersentinel;

// Global agent instance for signal handling
static Agent* g_agent = nullptr;

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        Logger::info("Received shutdown signal");
        if (g_agent) {
            g_agent->stop();
        }
    }
}

// Windows console control handler
BOOL WINAPI console_ctrl_handler(DWORD ctrl_type) {
    switch (ctrl_type) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            Logger::info("Received Windows shutdown event");
            if (g_agent) {
                g_agent->stop();
            }
            return TRUE;
        default:
            return FALSE;
    }
}

int main(int argc, char* argv[]) {
    // Initialize logger
    Logger::initialize("cybersentinel_agent.log");
    Logger::info("=== CyberSentinel DLP Agent Starting ===");

    // Configuration file path
    std::string config_file = "agent_config.json";
    if (argc > 1) {
        config_file = argv[1];
    }

    Logger::info("Using configuration file: " + config_file);

    try {
        // Create agent
        Agent agent(config_file);
        g_agent = &agent;

        // Register signal handlers
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

        // Initialize agent
        if (!agent.initialize()) {
            Logger::error("Agent initialization failed");
            return 1;
        }

        // Run agent
        Logger::info("Agent is now running. Press Ctrl+C to stop.");
        agent.run();

        Logger::info("=== CyberSentinel DLP Agent Stopped ===");
        return 0;

    } catch (const std::exception& e) {
        Logger::error(std::string("Fatal error: ") + e.what());
        return 1;
    } catch (...) {
        Logger::error("Unknown fatal error occurred");
        return 1;
    }
}
