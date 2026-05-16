#pragma once

#include "protocols/http/api_service.h"
#include "external/nlohmann/json.hpp"

namespace iot::protocols::http {

    class WarehouseApiService : public BaseApiService {
    public:
        explicit WarehouseApiService(const std::string& baseUrl);

        // GET methods
        HttpResponse listSensors();
        HttpResponse listDevices();
        HttpResponse listLines();
        HttpResponse getLicenseInfo(const std::string& roomId);
        HttpResponse getLocalRoom();
        HttpResponse getAgrowtekScanResult();

        // POST/PUT methods
        HttpResponse resetDeviceDone(const nlohmann::json& payload);
        HttpResponse sendSensorData(const nlohmann::json& payload);
        HttpResponse sendDeviceStatus(const nlohmann::json& payload);
        HttpResponse resetSensorDone(const nlohmann::json& payload);
        HttpResponse updateFirmwareVersion(const nlohmann::json& payload);
        HttpResponse updateCalibrationDone(const nlohmann::json& payload);
        HttpResponse startAgrowtekScan();

    private:
        // Helper to convert json to string for base methods
        HttpResponse postJson(const std::string& endpoint, const nlohmann::json& json);
        HttpResponse putJson(const std::string& endpoint, const nlohmann::json& json);
    };

} // namespace iot::protocols::http
