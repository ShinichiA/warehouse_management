#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <array>

namespace iot {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

/**
 * @brief Thread-safe Logger class for the IoT system.
 * Implements a singleton pattern to provide a centralized logging mechanism.
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance of the Logger.
     * @return Logger& Reference to the Logger instance.
     */
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief Log a message with specified level and source information.
     * @param level The importance level of the log message.
     * @param message The content of the log.
     * @param file The source file where the log originated.
     * @param line The line number where the log originated.
     */
    void log(const LogLevel level, const std::string& message, const char* file, const int line) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!enabledLevels_[levelToIndex(level)]) {
            return;
        }

        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm tm_struct;
        localtime_r(&time_t, &tm_struct);

        std::cout << "[" << std::put_time(&tm_struct, "%Y-%m-%d %H:%M:%S") 
                  << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
                  << "[" << levelToString(level) << "] "
                  << "[" << file << ":" << line << "] "
                  << message << std::endl;
    }

    void setLevelEnabled(const LogLevel level, const bool enabled) {
        std::lock_guard<std::mutex> lock(mutex_);
        enabledLevels_[levelToIndex(level)] = enabled;
    }

    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    std::mutex mutex_;
    std::array<bool, 4> enabledLevels_{true, true, true, true};

    /**
     * @brief Convert LogLevel enum to a string representation.
     */
    static const char* levelToString(const LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG:   return "DEBUG";
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARN";
            case LogLevel::ERROR:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }

    static std::size_t levelToIndex(const LogLevel level) {
        return static_cast<std::size_t>(level);
    }
};

} // namespace iot

/**
 * @brief Macro to log a message at DEBUG level.
 */
#define LOG_DEBUG(msg)   iot::Logger::getInstance().log(iot::LogLevel::DEBUG, msg, __FILE__, __LINE__)

/**
 * @brief Macro to log a message at INFO level.
 */
#define LOG_INFO(msg)    iot::Logger::getInstance().log(iot::LogLevel::INFO, msg, __FILE__, __LINE__)

/**
 * @brief Macro to log a message at WARNING level.
 */
#define LOG_WARNING(msg) iot::Logger::getInstance().log(iot::LogLevel::WARNING, msg, __FILE__, __LINE__)

/**
 * @brief Macro to log a message at ERROR level.
 */
#define LOG_ERROR(msg)   iot::Logger::getInstance().log(iot::LogLevel::ERROR, msg, __FILE__, __LINE__)
