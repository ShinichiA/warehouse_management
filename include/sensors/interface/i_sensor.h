#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace iot {

/**
 * @brief Enum for unit types (Key).
 */
enum class UnitType : uint8_t {
    NONE = 0,
    CELSIUS = 1,
    FAHRENHEIT = 2,
    KELVIN = 3,
    PERCENTAGE = 4,
    HUMIDITY = 5,
    PRESSURE = 6
};

/**
 * @brief Structure to represent a measurement unit (Value).
 */
struct SensorUnit {
    UnitType type;
    std::string name;
    std::string symbol;

    static SensorUnit fromType(const UnitType type) {
        switch (type) {
            case UnitType::CELSIUS:    return {UnitType::CELSIUS, "Celsius", "°C"};
            case UnitType::FAHRENHEIT: return {UnitType::FAHRENHEIT, "Fahrenheit", "°F"};
            case UnitType::KELVIN:     return {UnitType::KELVIN, "Kelvin", "K"};
            case UnitType::PERCENTAGE: return {UnitType::PERCENTAGE, "Percentage", "%"};
            case UnitType::HUMIDITY:   return {UnitType::HUMIDITY, "Humidity", "%RH"};
            case UnitType::PRESSURE:   return {UnitType::PRESSURE, "Pressure", "Pa"};
            default:                   return {UnitType::NONE, "Unknown", ""};
        }
    }
};

/**
 * @brief Enum for sensor data types.
 */
enum class DataType : uint8_t {
    INT16,
    UINT16,
    INT32,
    UINT32,
    FLOAT
};

/**
 * @brief Configuration for a single register mapping.
 */
struct RegisterConfig {
    uint16_t address{};
    DataType dataType{};
    UnitType unitType{};
    float scale = 1.0f;
    float offset = 0.0f;
};

/**
 * @brief Structure to represent a single measurement reading.
 */
struct Reading {
    UnitType type;
    float value;
    SensorUnit unit;

    Reading(UnitType t, float v) : type(t), value(v), unit(SensorUnit::fromType(t)) {}
};

/**
 * @brief ISensor Interface.
 * All sensor implementations must inherit from this.
 */
class ISensor {
public:
    virtual ~ISensor() = default;

    /**
     * @brief Initialize the sensor.
     * @return true if successful
     */
    virtual bool initialize() = 0;

    /**
     * @brief Read all configured values from the sensor.
     * @return std::vector<Reading> List of readings
     */
    virtual std::vector<Reading> readAll() = 0;

    /**
     * @brief Get the sensor name/ID.
     */
    [[nodiscard]] virtual std::string getName() const = 0;
};

} // namespace iot
