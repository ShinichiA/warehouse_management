#include "protocols/http/warehouse_api_service.h"

namespace iot::protocols::http {

WarehouseApiService::WarehouseApiService(const std::string& baseUrl) 
    : BaseApiService(baseUrl) {}

HttpResponse WarehouseApiService::listSensors() {
    return get("/api/v1/firmware/sensors/");
}

HttpResponse WarehouseApiService::listDevices() {
    return get("/api/v1/firmware/devices/");
}

HttpResponse WarehouseApiService::listLines() {
    return get("/api/v1/firmware/lines/");
}

HttpResponse WarehouseApiService::getLicenseInfo(const std::string& roomId) {
    return get("/api/v1/license/license-info/" + roomId + "/");
}

HttpResponse WarehouseApiService::getLocalRoom() {
    return get("/api/v1/rooms/local-room/");
}

HttpResponse WarehouseApiService::getAgrowtekScanResult() {
    return get("/api/v1/firmware/agrowtek/scan/result/");
}

HttpResponse WarehouseApiService::resetDeviceDone(const nlohmann::json& payload) {
    return postJson("/api/v1/firmware/devices/reset-done/", payload);
}

HttpResponse WarehouseApiService::sendSensorData(const nlohmann::json& payload) {
    return postJson("/api/v1/sensor-data/", payload);
}

HttpResponse WarehouseApiService::sendDeviceStatus(const nlohmann::json& payload) {
    return postJson("/api/v1/device-status/", payload);
}

HttpResponse WarehouseApiService::resetSensorDone(const nlohmann::json& payload) {
    return postJson("/api/v1/firmware/sensors/reset-done/", payload);
}

HttpResponse WarehouseApiService::updateFirmwareVersion(const nlohmann::json& payload) {
    return postJson("/api/v1/versions/firmware-version/", payload);
}

HttpResponse WarehouseApiService::updateCalibrationDone(const nlohmann::json& payload) {
    return postJson("/api/v1/calibration/done/", payload);
}

HttpResponse WarehouseApiService::startAgrowtekScan() {
    return postJson("/api/v1/firmware/agrowtek/scan/start/", {});
}

HttpResponse WarehouseApiService::postJson(const std::string& endpoint, const nlohmann::json& json) {
    return post(endpoint, json.dump());
}

HttpResponse WarehouseApiService::putJson(const std::string& endpoint, const nlohmann::json& json) {
    return put(endpoint, json.dump());
}

} // namespace iot::protocols::http
