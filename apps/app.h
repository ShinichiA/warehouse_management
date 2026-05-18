#pragma once
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <string>
#include "protocols/mqtt/interface/i_mqtt_client.h"
#include "sensors/interface/i_sensor.h"
#include "protocols/modbus/modbus_rtu.h"
#include "protocols/http/warehouse_api_service.h"


namespace iot {

class App {
public:
    App();
    ~App();

    // Disallow copying to prevent threading issues
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    /**
     * @brief Initialize all application subsystems (Modbus, Sensors, MQTT).
     * @param configPath Path to the JSON configuration file.
     * @return true if initialization was successful
     */
    bool initialize(const char *configPath = "config/config.json");

    /**
     * @brief Start the main execution loop in a background thread.
     * @param maxSensorCycles Optional limit on number of reading cycles (0 for infinite).
     */
    void run(int maxSensorCycles = 0);

    /**
     * @brief Stop the background thread and cleanup resources.
     */
    void shutdown();

private:
    static void printBanner();
    void workerTask() const; // The background thread logic

    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    
    std::shared_ptr<ModbusRTU> modbus_;
    std::shared_ptr<IMqttClient> mqtt_;
    std::shared_ptr<protocols::http::WarehouseApiService> api_;
    std::vector<std::shared_ptr<ISensor>> sensors_;

    std::string mqttClientId_{"iot_warehouse_gate"};
    std::string mqttHost_{"localhost"};
    int mqttPort_{1883};
    int mqttKeepAlive_{60};
    std::string mqttTopicPrefix_{"warehouse/sensors"};
    std::string apiBaseUrl_{"http://localhost:8002"};
    std::string modbusDevice_{"/dev/ttyACM0"};
    int modbusBaudRate_{9600};
    char modbusParity_{'N'};
    int modbusDataBits_{8};
    int modbusStopBits_{1};
    
    std::thread workerThread_;
};

} // namespace iot
