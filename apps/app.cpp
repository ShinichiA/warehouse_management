#include "app.h"
#include <chrono>
#include "protocols/mqtt/mosquitto_client.h"
#include "sensors/SHT35.h"
#include "utils/helper.h"
#include "utils/logger.h"

namespace iot {

App::App() = default;

App::~App() {
    shutdown();
}

bool App::initialize(const char *configPath) {
    printBanner();
    LOG_INFO("Initializing application subsystems...");

    // 1. Setup Modbus
    modbus_ = std::make_shared<ModbusRTU>("/dev/ttyACM0", 9600, 'N', 8, 1);
    if (!modbus_->connect()) {
        LOG_ERROR("Failed to initialize Modbus on /dev/ttyACM0");
        return false;
    }

    // 2. Setup multiple sensors
    // In a real app, this part would read from config/config.json
    
    // Sensor 1: Reading Temperature (INT16, addr 0) and Humidity (INT16, addr 1)
    std::vector<RegisterConfig> config1 = {
        {0, DataType::INT16, UnitType::CELSIUS, 0.1f},
        {1, DataType::INT16, UnitType::HUMIDITY, 0.1f}
    };
    auto s1 = std::make_shared<SHT35>(modbus_, 1, config1);

    // Sensor 2: Reading Temperature (FLOAT, addr 10) and Pressure (INT32, addr 20)
    std::vector<RegisterConfig> config2 = {
        {10, DataType::FLOAT, UnitType::CELSIUS, 1.0f},
        {20, DataType::INT32, UnitType::PRESSURE, 1.0f}
    };
    auto s2 = std::make_shared<SHT35>(modbus_, 2, config2);

    if (s1->initialize()) sensors_.push_back(s1);
    if (s2->initialize()) sensors_.push_back(s2);

    LOG_INFO("Registered " + std::to_string(sensors_.size()) + " sensors.");

    // 3. Setup MQTT
    mqtt_ = std::make_shared<MosquittoClient>("iot_warehouse_gate");
    if (!mqtt_->connect("localhost", 1883, 60)) {
        LOG_WARNING("MQTT Broker not found. Will retry later.");
    }

    this->initialized_ = true;
    return true;
}

void App::run(int maxSensorCycles) {
    if (!this->initialized_) {
        LOG_ERROR("Cannot run: App not initialized!");
        return;
    }

    LOG_INFO("Starting background worker thread...");
    running_ = true;
    workerThread_ = std::thread(&App::workerTask, this);
}

void App::workerTask() const {
    LOG_INFO("Worker thread started.");

    while (running_) {
        if (!mqtt_->isConnected()) {
            mqtt_->connect("localhost", 1883, 60);
        }

        // Loop through all registered sensors
        for (const auto& sensor : sensors_) {
            auto readings = sensor->readAll();
            
            if (mqtt_->isConnected()) {
                for (const auto& r : readings) {
                    std::string payload = utils::format(
                        R"({"sensor": "%s", "type": "%s", "value": %.2f, "unit": "%s"})",
                        sensor->getName().c_str(), r.unit.name.c_str(), r.value, r.unit.symbol.c_str()
                    );
                    
                    // Publish each reading to its own topic
                    std::string topic = utils::format("warehouse/sensors/%s/%s", 
                                                     sensor->getName().c_str(), 
                                                     r.unit.name.c_str());
                    mqtt_->publish(topic, payload, 1, false);
                    LOG_DEBUG(utils::format("Published: %s", payload.c_str()));
                }
            }
        }

        utils::sleep_ms(5000); // Interval for all sensors
    }
    LOG_INFO("Worker thread stopping...");
}

void App::printBanner() {
    LOG_INFO("========================================");
    LOG_INFO("   IOT WAREHOUSE MANAGEMENT SYSTEM      ");
    LOG_INFO("========================================");
}

void App::shutdown() {
    if (running_) {
        LOG_INFO("Stopping worker thread...");
        running_ = false;
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }

    if (this->initialized_) {
        LOG_INFO("Shutting down subsystems...");
        if (mqtt_) mqtt_->disconnect();
        if (modbus_) modbus_->disconnect();
        this->initialized_ = false;
    }
}

} // namespace iot
