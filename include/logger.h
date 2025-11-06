#ifndef CYBERSENTINEL_LOGGER_H
#define CYBERSENTINEL_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

namespace cybersentinel {

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static void init(const std::string& log_file = "cybersentinel_agent.log");
    static void set_level(Level level);

    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);

private:
    static void log(Level level, const std::string& message);
    static std::string level_to_string(Level level);
    static std::string get_timestamp();

    static std::ofstream log_file_;
    static std::mutex mutex_;
    static Level min_level_;
};

} // namespace cybersentinel

#endif // CYBERSENTINEL_LOGGER_H
