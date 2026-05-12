#pragma once

#include <cstdint>
#include <cstddef>
#include "utils/helper.h"

namespace iot {

/**
 * @brief Utility for Modbus CRC16 calculation.
 */
class CRC16 {
public:
    /**
     * @brief Calculate CRC16 for Modbus RTU.
     * @param data Pointer to data array
     * @param length Length of data
     * @return uint16_t Calculated CRC (LSB first)
     */
    static uint16_t calculate(const uint8_t* data, size_t length) {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= utils::cast<uint16_t>(data[i]);
            for (int j = 8; j != 0; j--) {
                if ((crc & 0x0001) != 0) {
                    crc >>= 1;
                    crc ^= 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    /**
     * @brief Append CRC16 to a buffer.
     * @param buffer Pointer to buffer (must have space for 2 extra bytes)
     * @param length Current length of data in buffer
     * @return New length including CRC
     */
    static size_t append(uint8_t* buffer, size_t length) {
        uint16_t crc = calculate(buffer, length);
        buffer[length] = utils::cast<uint8_t>(crc & 0xFF);         // LSB
        buffer[length + 1] = utils::cast<uint8_t>(crc >> 8);      // MSB
        return length + 2;
    }

    /**
     * @brief Verify CRC16 of a received packet.
     */
    static bool verify(const uint8_t* buffer, size_t length) {
        if (length < 3) return false;
        uint16_t calculated = calculate(buffer, length - 2);
        uint16_t received = utils::cast<uint16_t>(buffer[length - 2]) | 
                           (utils::cast<uint16_t>(buffer[length - 1]) << 8);
        return calculated == received;
    }
};

} // namespace iot
