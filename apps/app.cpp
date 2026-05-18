#include "app.h"
#include <chrono>
#include "protocols/mqtt/mosquitto_client.h"
#include "sensors/SHT35.h"
#include "utils/helper.h"
#include "utils/logger.h"
#include "protocols/http/warehouse_api_service.h"
#include "external/nlohmann/json.hpp"
#include <fstream>

namespace iot {

App::App() = default;

App::~App() {
    shutdown();
}

bool App::initialize(const char *configPath) {
    printBanner();
    LOG_INFO("Initializing application subsystems...");

    // Load runtime configuration
    try {
        std::ifstream configFile(configPath);
        if (configFile.is_open()) {
            nlohmann::json cfg = nlohmann::json::parse(configFile, nullptr, true, true);

            if (cfg.contains("modbus")) {
                const auto& modbusCfg = cfg["modbus"];
                modbusDevice_ = modbusCfg.value("device", modbusDevice_);
                modbusBaudRate_ = modbusCfg.value("baud_rate", modbusBaudRate_);
                const auto parity = modbusCfg.value("parity", std::string(1, modbusParity_));
                if (!parity.empty()) {
                    modbusParity_ = parity[0];
                }
                modbusDataBits_ = modbusCfg.value("data_bits", modbusDataBits_);
                modbusStopBits_ = modbusCfg.value("stop_bits", modbusStopBits_);
            }

            if (cfg.contains("mqtt")) {
                const auto& mqttCfg = cfg["mqtt"];
                mqttClientId_ = mqttCfg.value("client_id", mqttClientId_);
                mqttHost_ = mqttCfg.value("host", mqttHost_);
                mqttPort_ = mqttCfg.value("port", mqttPort_);
                mqttKeepAlive_ = mqttCfg.value("keep_alive", mqttKeepAlive_);
                mqttTopicPrefix_ = mqttCfg.value("topic_prefix", mqttTopicPrefix_);
            }

            if (cfg.contains("api")) {
                const auto& apiCfg = cfg["api"];
                apiBaseUrl_ = apiCfg.value("base_url", apiBaseUrl_);
            }

            if (cfg.contains("system") && cfg["system"].contains("log_enabled")) {
                const auto& logEnabled = cfg["system"]["log_enabled"];
                Logger::getInstance().setLevelEnabled(LogLevel::DEBUG, logEnabled.value("debug", true));
                Logger::getInstance().setLevelEnabled(LogLevel::INFO, logEnabled.value("info", true));
                Logger::getInstance().setLevelEnabled(LogLevel::WARNING, logEnabled.value("warning", true));
                Logger::getInstance().setLevelEnabled(LogLevel::ERROR, logEnabled.value("error", true));
            }
        } else {
            LOG_WARNING(utils::format("Config file not found: %s. Using defaults.", configPath));
        }
    } catch (const std::exception& ex) {
        LOG_ERROR(utils::format("Invalid config file: %s. Error: %s. Exiting.", configPath, ex.what()));
        return false;
    }

    // 1. Setup Modbus
    modbus_ = std::make_shared<ModbusRTU>(modbusDevice_, modbusBaudRate_, modbusParity_, modbusDataBits_, modbusStopBits_);
    if (!modbus_->connect()) {
        LOG_ERROR("Failed to initialize Modbus on " + modbusDevice_);
        return false;
    }

    // 2. Setup multiple sensors
    auto s1 = std::make_shared<SHT35>(modbus_, 1); // Use default config
    auto s2 = std::make_shared<SHT35>(modbus_, 2); // Use default config

    if (s1->initialize()) sensors_.push_back(s1);
    if (s2->initialize()) sensors_.push_back(s2);

    LOG_INFO("Registered " + std::to_string(sensors_.size()) + " sensors.");

    // 3. Setup MQTT
    mqtt_ = std::make_shared<MosquittoClient>(mqttClientId_);
    if (!mqtt_->connect(mqttHost_, mqttPort_, mqttKeepAlive_)) {
        LOG_WARNING("MQTT Broker not found. Will retry later.");
    }

    // 4. Setup HTTP API Client
    api_ = std::make_shared<protocols::http::WarehouseApiService>(apiBaseUrl_);
    LOG_INFO("HTTP API Client initialized with base URL: " + apiBaseUrl_);
    auto res = api_->getLicenseInfo("d97dff02-2bc7-4d36-a163-70edc82e4697");
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
            mqtt_->connect(mqttHost_, mqttPort_, mqttKeepAlive_);
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
                    std::string topic = utils::format("%s/%s/%s",
                                                     mqttTopicPrefix_.c_str(),
                                                     sensor->getName().c_str(), 
                                                     r.unit.name.c_str());
                    mqtt_->publish(topic, payload, 1, false);
                    LOG_DEBUG(utils::format("MQTT Published: %s", payload.c_str()));

                    // Also send via HTTP API
                    nlohmann::json jsonPayload = nlohmann::json::parse(payload);
                    auto resp = api_->sendSensorData(jsonPayload);
                    if (resp.status_code == 200 || resp.status_code == 201) {
                        LOG_DEBUG("HTTP API: Data sent successfully.");
                    } else {
                        LOG_WARNING(utils::format("HTTP API Error: %ld - %s", resp.status_code, resp.error.c_str()));
                    }
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
