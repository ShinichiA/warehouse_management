#pragma once
#include "sensors/interface/i_sensor.h"
#include "protocols/modbus/modbus_rtu.h"
#include <memory>

namespace iot {

/**
 * @brief SHT35 Sensor implementation using Modbus RTU.
 */
class SHT35 : public ISensor {
public:
    /**
     * @brief Construct a new SHT35 sensor with fixed register mapping.
     * @param modbus Shared Modbus RTU master instance.
     * @param slaveId Modbus slave ID of the sensor.
     */
    SHT35(std::shared_ptr<ModbusRTU> modbus, int slaveId);
    
    ~SHT35() override = default;

    /**
     * @brief Initialize the sensor and calculate register range.
     * @return true if successful
     */
    bool initialize() override;

    /**
     * @brief Read all configured registers in one Modbus call and parse them.
     * @return std::vector<Reading> List of readings
     */
    std::vector<Reading> readAll() override;

    /**
     * @brief Get the display name of the sensor.
     */
    [[nodiscard]] std::string getName() const override;

private:
    /**
     * @brief Default register mapping for SHT35.
     */
    static const std::vector<RegisterConfig> DEFAULT_CONFIGS;

    std::shared_ptr<ModbusRTU> modbus_;
    int slaveId_;

    uint16_t startAddress_ = 0;
    uint16_t totalRegisters_ = 0;
};

} // namespace iot
