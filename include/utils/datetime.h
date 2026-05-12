#pragma once

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace iot {
    class DateTimeUtils {
    public:
        /**
         * @brief Get current local time as a formatted string.
         * @param format strftime format string
         * @return Formatted date-time string
         */
        static std::string nowStr(const std::string &format = "%Y-%m-%d %H:%M:%S") {
            const auto now = std::chrono::system_clock::now();
            return toString(now, format);
        }

        /**
         * @brief Get current timestamp in milliseconds.
         * @return Milliseconds since epoch
         */
        static int64_t currentTimestampMs() {
            const auto now = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count();
        }

        /**
         * @brief Convert a chrono time point to a formatted string.
         * @param timePoint Chrono system_clock time point
         * @param format strftime format string
         * @return Formatted date-time string
         */
        static std::string toString(const std::chrono::system_clock::time_point &timePoint,
                                    const std::string &format = "%Y-%m-%d %H:%M:%S") {
            const std::time_t timeT = std::chrono::system_clock::to_time_t(timePoint);
            std::tm tmStruct;
            localtime_r(&timeT, &tmStruct); // Thread-safe on Linux

            std::ostringstream oss;
            oss << std::put_time(&tmStruct, format.c_str());
            return oss.str();
        }

        /**
         * @brief Convert milliseconds timestamp to formatted string.
         * @param timestampMs Milliseconds since epoch
         * @param format strftime format string
         * @return Formatted date-time string
         */
        static std::string fromTimestamp(
            const int64_t timestampMs,
            const std::string &format = "%Y-%m-%d %H:%M:%S") {
            const auto tp = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(timestampMs)
            );
            return toString(tp, format);
        }

        /**
         * @brief Get current date string (YYYY-MM-DD).
         */
        static std::string currentDate() {
            return nowStr("%Y-%m-%d");
        }

        /**
         * @brief Get current time string (HH:MM:SS).
         */
        static std::string currentTime() {
            return nowStr("%H:%M:%S");
        }
    };
} // namespace iot
