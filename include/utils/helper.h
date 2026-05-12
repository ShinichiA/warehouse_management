#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <type_traits>

namespace iot::utils {

    /**
     * @brief Utility functions used frequently across the system.
     */

    /**
     * @brief Sleep for a specified number of milliseconds.
     * @param ms Milliseconds to sleep.
     */
    inline void sleep_ms(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

    /**
     * @brief Sleep for a specified number of seconds.
     * @param sec Seconds to sleep.
     */
    inline void sleep_sec(int sec) { std::this_thread::sleep_for(std::chrono::seconds(sec)); }

    /**
     * @brief Check if a vector contains a specific value.
     * @param vec The vector to search in.
     * @param value The value to search for.
     * @return true if the value is found.
     */
    template<typename T>
    inline bool contains(const std::vector<T> &vec, const T &value) {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }

    /**
     * @brief Check if a string contains a specific substring.
     * @param str The string to search in.
     * @param substr The substring to search for.
     * @return true if the substring is found.
     */
    inline bool contains(const std::string &str, const std::string &substr) {
        return str.find(substr) != std::string::npos;
    }

    /**
     * @brief Convert an enum class to its underlying integral type.
     * @tparam E Enum type.
     * @param e Enum value.
     * @return The underlying integral value.
     */
    template<typename E>
    constexpr auto to_underlying(E e) noexcept {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    /**
     * @brief Cast a pointer from one type to another (shorthand for static_cast).
     * @tparam T Target pointer type.
     * @tparam U Source pointer type.
     * @param ptr Pointer to cast.
     * @return Casted pointer.
     */
    template<typename T, typename U>
    constexpr T *as(U *ptr) noexcept {
        return static_cast<T *>(ptr);
    }

    /**
     * @brief General purpose static_cast wrapper.
     * @tparam T Target type.
     * @tparam U Source type.
     * @param val Value to cast.
     * @return Casted value.
     */
    template<typename T, typename U>
    constexpr T cast(U val) noexcept {
        return static_cast<T>(val);
    }

    /**
     * @brief Clamp a value between a minimum and maximum.
     * @param v Value to clamp.
     * @param lo Lower bound.
     * @param hi Upper bound.
     * @return Clamped value.
     */
    template<typename T>
    constexpr const T &clamp(const T &v, const T &lo, const T &hi) {
        return (v < lo) ? lo : (hi < v) ? hi : v;
    }

    /**
     * @brief Format a string (printf style).
     * @param fmt Format string.
     * @param args Arguments for formatting.
     * @return Formatted string.
     */
    template<typename... Args>
    inline std::string format(const char *fmt, Args... args) {
        size_t size = snprintf(nullptr, 0, fmt, args...) + 1;
        char *buf = new char[size];
        snprintf(buf, size, fmt, args...);
        std::string res(buf);
        delete[] buf;
        return res;
    }

} // namespace iot::utils
