#include "protocols/modbus/modbus_rtu.h"
#include "utils/logger.h"
#include "utils/crc16.h"
#include "utils/helper.h"
#include <cerrno>
#include <cstring>

namespace iot {

ModbusRTU::ModbusRTU(const std::string& device, int baud, char parity, int data_bit, int stop_bit)
    : device_(device), baud_(baud), parity_(parity), data_bit_(data_bit), stop_bit_(stop_bit) {}

ModbusRTU::~ModbusRTU() {
    disconnect();
}

bool ModbusRTU::connect() {
    std::lock_guard<std::mutex> lock(mtx_);
    ctx_ = modbus_new_rtu(device_.c_str(), baud_, parity_, data_bit_, stop_bit_);
    if (!ctx_) {
        LOG_ERROR("Modbus: Failed to create context for " + device_);
        return false;
    }

    if (modbus_connect(ctx_) == -1) {
        LOG_ERROR("Modbus: Connection failed on " + device_ + ": " + modbus_strerror(errno));
        modbus_free(ctx_);
        ctx_ = nullptr;
        return false;
    }

    // Apply the configured timeout
    modbus_set_response_timeout(ctx_, response_timeout_sec_, response_timeout_usec_);

    connected_ = true;
    LOG_INFO("Modbus: Connected to " + device_ + " (Timeout: " + 
             std::to_string(response_timeout_sec_) + "s " + 
             std::to_string(response_timeout_usec_) + "us)");
    return true;
}

void ModbusRTU::disconnect() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (ctx_) {
        modbus_close(ctx_);
        modbus_free(ctx_);
        ctx_ = nullptr;
        connected_ = false;
        LOG_INFO("Modbus: Disconnected " + device_);
    }
}

bool ModbusRTU::isConnected() const {
    return connected_;
}

void ModbusRTU::setTimeout(const uint32_t sec, const uint32_t usec) {
    std::lock_guard<std::mutex> lock(mtx_);
    response_timeout_sec_ = sec;
    response_timeout_usec_ = usec;
    
    if (ctx_ && connected_) {
        modbus_set_response_timeout(ctx_, response_timeout_sec_, response_timeout_usec_);
        LOG_INFO("Modbus: Timeout updated to " + std::to_string(sec) + "s " + std::to_string(usec) + "us");
    }
}

int ModbusRTU::sendRawPacket(const uint8_t* raw_req, const int req_len, uint8_t* rsp_buf) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ctx_ || !connected_) return -1;

    // 1. Calculate and append CRC16 (Libmodbus raw send requires full packet)
    uint8_t final_req[MODBUS_RTU_MAX_ADU_LENGTH];
    memcpy(final_req, raw_req, req_len);
    const size_t final_len = CRC16::append(final_req, req_len);

    // 2. Send the raw request
    int sent = modbus_send_raw_request(ctx_, final_req, utils::cast<int>(final_len));
    if (sent == -1) {
        LOG_ERROR("Modbus Raw: Failed to send packet: " + std::string(modbus_strerror(errno)));
        return -1;
    }

    // 3. Receive the confirmation (response)
    int rsp_len = modbus_receive_confirmation(ctx_, rsp_buf);
    if (rsp_len == -1) {
        LOG_ERROR("Modbus Raw: Failed to receive response: " + std::string(modbus_strerror(errno)));
        return -1;
    }

    // 4. Verify CRC of response
    if (!CRC16::verify(rsp_buf, rsp_len)) {
        LOG_ERROR("Modbus Raw: CRC verification failed for response!");
        return -1;
    }

    return rsp_len;
}

bool ModbusRTU::readCoils(const int slaveId, const int addr, const int nb, uint8_t* dest) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ctx_ || !connected_) return false;

    modbus_set_slave(ctx_, slaveId);
    if (modbus_read_bits(ctx_, addr, nb, dest) == -1) {
        LOG_ERROR("Modbus Error (Read Coils Slave " + std::to_string(slaveId) + "): " + modbus_strerror(errno));
        return false;
    }
    return true;
}

bool ModbusRTU::writeCoil(const int slaveId, const int addr, const bool status) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ctx_ || !connected_) return false;

    modbus_set_slave(ctx_, slaveId);
    if (modbus_write_bit(ctx_, addr, status ? 1 : 0) == -1) {
        LOG_ERROR("Modbus Error (Write Coil Slave " + std::to_string(slaveId) + "): " + modbus_strerror(errno));
        return false;
    }
    return true;
}

bool ModbusRTU::writeCoils(const int slaveId, const int addr, const int nb, const uint8_t* src) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ctx_ || !connected_) return false;

    modbus_set_slave(ctx_, slaveId);
    if (modbus_write_bits(ctx_, addr, nb, src) == -1) {
        LOG_ERROR("Modbus Error (Write Coils Slave " + std::to_string(slaveId) + "): " + modbus_strerror(errno));
        return false;
    }
    return true;
}

bool ModbusRTU::readDiscreteInputs(const int slaveId, const int addr, const int nb, uint8_t* dest) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ctx_ || !connected_) return false;

    modbus_set_slave(ctx_, slaveId);
    if (modbus_read_input_bits(ctx_, addr, nb, dest) == -1) {
        LOG_ERROR("Modbus Error (Read Discrete Inputs Slave " + std::to_string(slaveId) + "): " + modbus_strerror(errno));
        return false;
    }
    return true;
}

bool ModbusRTU::readHoldingRegisters(const int slaveId, const int addr, const int nb, uint16_t* dest) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ctx_ || !connected_) return false;

    modbus_set_slave(ctx_, slaveId);
    if (modbus_read_registers(ctx_, addr, nb, dest) == -1) {
        LOG_ERROR("Modbus Error (Read Holding Regs Slave " + std::to_string(slaveId) + "): " + modbus_strerror(errno));
        return false;
    }
    return true;
}

bool ModbusRTU::writeRegister(const int slaveId, const int addr, const uint16_t value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ctx_ || !connected_) return false;

    modbus_set_slave(ctx_, slaveId);
    if (modbus_write_register(ctx_, addr, value) == -1) {
        LOG_ERROR("Modbus Error (Write Register Slave " + std::to_string(slaveId) + "): " + modbus_strerror(errno));
        return false;
    }
    return true;
}

bool ModbusRTU::writeRegisters(const int slaveId, const int addr, const int nb, const uint16_t* src) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ctx_ || !connected_) return false;

    modbus_set_slave(ctx_, slaveId);
    if (modbus_write_registers(ctx_, addr, nb, src) == -1) {
        LOG_ERROR("Modbus Error (Write Registers Slave " + std::to_string(slaveId) + "): " + modbus_strerror(errno));
        return false;
    }
    return true;
}

bool ModbusRTU::readInputRegisters(const int slaveId, const int addr, const int nb, uint16_t* dest) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!ctx_ || !connected_) return false;

    modbus_set_slave(ctx_, slaveId);
    if (modbus_read_input_registers(ctx_, addr, nb, dest) == -1) {
        LOG_ERROR("Modbus Error (Read Input Regs Slave " + std::to_string(slaveId) + "): " + modbus_strerror(errno));
        return false;
    }
    return true;
}

} // namespace iot
