#pragma once

#include <modbus.h>
#include <string>
#include <mutex>

namespace iot {

/**
 * @brief Generic Modbus Master (Client) Protocol Handler.
 * This is the shared component that handles all Modbus communication.
 */
class ModbusRTU {
public:
    /**
     * @brief Construct a new Modbus RTU object.
     * @param device Serial device path (e.g., "/dev/ttyUSB0")
     * @param baud Baud rate (e.g., 9600)
     * @param parity Parity ('N', 'E', 'O')
     * @param data_bit Data bits (usually 8)
     * @param stop_bit Stop bits (usually 1 or 2)
     */
    ModbusRTU(const std::string& device, int baud, char parity, int data_bit, int stop_bit);
    
    ~ModbusRTU();

    /**
     * @brief Establish connection to the serial device.
     * @return true if successful
     */
    bool connect();

    /**
     * @brief Close the serial connection and free context.
     */
    void disconnect();

    /**
     * @brief Check if the Modbus connection is active.
     */
    [[nodiscard]] bool isConnected() const;

    /**
     * @brief Set the response timeout for Modbus operations.
     * @param sec Seconds
     * @param usec Microseconds
     */
    void setTimeout(uint32_t sec, uint32_t usec);

    /**
     * @brief Send a custom raw packet and wait for response.
     * Use this to bypass library logic and build your own packets.
     * @param raw_req The request bytes (should NOT include CRC, it will be added)
     * @param req_len Length of request
     * @param rsp_buf Buffer to store response
     * @return int Length of response received, or -1 on error
     */
    int sendRawPacket(const uint8_t* raw_req, int req_len, uint8_t* rsp_buf);

    /**
     * @brief Read coil statuses from a slave device.
     */
    bool readCoils(int slaveId, int addr, int nb, uint8_t* dest);

    /**
     * @brief Write a single coil status.
     */
    bool writeCoil(int slaveId, int addr, bool status);

    /**
     * @brief Write multiple coil statuses.
     */
    bool writeCoils(int slaveId, int addr, int nb, const uint8_t* src);

    /**
     * @brief Read discrete input statuses.
     */
    bool readDiscreteInputs(int slaveId, int addr, int nb, uint8_t* dest);

    /**
     * @brief Read holding registers.
     */
    bool readHoldingRegisters(int slaveId, int addr, int nb, uint16_t* dest);

    /**
     * @brief Write a single holding register.
     */
    bool writeRegister(int slaveId, int addr, uint16_t value);

    /**
     * @brief Write multiple holding registers.
     */
    bool writeRegisters(int slaveId, int addr, int nb, const uint16_t* src);

    /**
     * @brief Read input registers.
     */
    bool readInputRegisters(int slaveId, int addr, int nb, uint16_t* dest);

private:
    modbus_t* ctx_ = nullptr;
    std::string device_;
    int baud_;
    int data_bit_;
    int stop_bit_;
    char parity_;
    bool connected_ = false;
    
    uint32_t response_timeout_sec_ = 1;
    uint32_t response_timeout_usec_ = 0;
    
    std::mutex mtx_; // Modbus RTU is half-duplex, needs locking for shared access
};

} // namespace iot
