#include "sensors/SHT35.h"

#include <utility>
#include <cstring>
#include "utils/helper.h"
#include "utils/logger.h"

namespace iot {

SHT35::SHT35(std::shared_ptr<ModbusRTU> modbus, int slaveId, std::vector<RegisterConfig> configs)
    : modbus_(std::move(modbus)), configs_(std::move(configs)), slaveId_(slaveId) {}

bool SHT35::initialize() {
    if (!modbus_) {
        LOG_ERROR("SHT35: Modbus master is null!");
        return false;
    }

    if (configs_.empty()) {
        LOG_WARNING("SHT35: No register configurations provided.");
        return true;
    }

    // Calculate the range of registers to read in one go
    uint16_t minAddr = 0xFFFF;
    uint16_t maxAddr = 0;

    for (const auto& c : configs_) {
        if (c.address < minAddr) minAddr = c.address;
        
        uint16_t endAddr = c.address;
        if (c.dataType == DataType::INT32 || c.dataType == DataType::UINT32 || c.dataType == DataType::FLOAT) {
            endAddr += 1; // Takes 2 registers
        }
        
        if (endAddr > maxAddr) maxAddr = endAddr;
    }

    startAddress_ = minAddr;
    totalRegisters_ = maxAddr - minAddr + 1;

    LOG_INFO(utils::format("SHT35: Initialized Slave %d. Batch read: Addr %d, Len %d", 
             slaveId_, startAddress_, totalRegisters_));
    return true;
}

std::vector<Reading> SHT35::readAll() {
    std::vector<Reading> readings;
    if (!modbus_ || !modbus_->isConnected()) {
        LOG_ERROR("SHT35: Modbus not connected!");
        return readings;
    }

    std::vector<uint16_t> buffer(totalRegisters_);
    if (modbus_->readHoldingRegisters(slaveId_, startAddress_, totalRegisters_, buffer.data())) {
        for (const auto& c : configs_) {
            size_t offset = c.address - startAddress_;
            float raw_val = 0;

            switch (c.dataType) {
                case DataType::INT16:
                    raw_val = utils::cast<float>(utils::cast<int16_t>(buffer[offset]));
                    break;
                case DataType::UINT16:
                    raw_val = utils::cast<float>(buffer[offset]);
                    break;
                case DataType::FLOAT: {
                    if (offset + 1 < buffer.size()) {
                        uint32_t combined = (utils::cast<uint32_t>(buffer[offset]) << 16) | buffer[offset + 1];
                        std::memcpy(&raw_val, &combined, sizeof(float));
                    }
                    break;
                }
                case DataType::INT32: {
                    if (offset + 1 < buffer.size()) {
                        int32_t combined = (utils::cast<int32_t>(buffer[offset]) << 16) | buffer[offset + 1];
                        raw_val = utils::cast<float>(combined);
                    }
                    break;
                }
                case DataType::UINT32: {
                    if (offset + 1 < buffer.size()) {
                        uint32_t combined = (utils::cast<uint32_t>(buffer[offset]) << 16) | buffer[offset + 1];
                        raw_val = utils::cast<float>(combined);
                    }
                    break;
                }
            }

            float final_val = (raw_val * c.scale) + c.offset;
            readings.emplace_back(c.unitType, final_val);
            
            LOG_DEBUG(utils::format("SHT35 [%d]: %s = %.2f (Raw: %.2f)", 
                      slaveId_, SensorUnit::fromType(c.unitType).name.c_str(), final_val, raw_val));
        }
    } else {
        LOG_ERROR(utils::format("SHT35 [%d]: Failed to read batch registers!", slaveId_));
    }

    return readings;
}

std::string SHT35::getName() const {
    return "SHT35_" + std::to_string(slaveId_);
}

} // namespace iot
