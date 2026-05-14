#include "protocols/mqtt/hive_mqtt_client.h"
#include "utils/logger.h"
#include "utils/helper.h"
#include <cstring>

namespace iot {

HiveMqttClient::HiveMqttClient(const std::string& id) {
    mosquitto_lib_init();
    mosq_ = mosquitto_new(id.empty() ? nullptr : id.c_str(), true, this);
    if (mosq_) {
        mosquitto_connect_callback_set(mosq_, on_connect_wrapper);
        mosquitto_message_callback_set(mosq_, on_message_wrapper);
    }
}

HiveMqttClient::~HiveMqttClient() {
    disconnect();
    if (mosq_) {
        mosquitto_destroy(mosq_);
    }
    mosquitto_lib_cleanup();
}

bool HiveMqttClient::connect(const std::string& host, int port, int keepalive) {
    if (!mosq_) return false;
    
    // HiveMQ specific logic could go here (e.g. enforcing TLS if port is 8883)
    LOG_INFO("HiveMQ: Attempting connection to " + host + ":" + std::to_string(port));
    
    int rc = mosquitto_connect(mosq_, host.c_str(), port, keepalive);
    if (rc != MOSQ_ERR_SUCCESS) {
        LOG_ERROR("HiveMQ: Connect failed: " + std::string(mosquitto_strerror(rc)));
        return false;
    }

    mosquitto_loop_start(mosq_);
    return true;
}

void HiveMqttClient::disconnect() {
    if (mosq_) {
        mosquitto_disconnect(mosq_);
        mosquitto_loop_stop(mosq_, false);
        connected_ = false;
    }
}

bool HiveMqttClient::isConnected() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return connected_;
}

void HiveMqttClient::setCredentials(const std::string& username, const std::string& password) {
    if (mosq_) {
        mosquitto_username_pw_set(mosq_, username.c_str(), password.c_str());
    }
}

void HiveMqttClient::setTls(const std::string& caPath, const std::string& certPath, const std::string& keyPath) {
    if (mosq_) {
        const char* cPath = certPath.empty() ? nullptr : certPath.c_str();
        const char* kPath = keyPath.empty() ? nullptr : keyPath.c_str();
        mosquitto_tls_set(mosq_, caPath.c_str(), nullptr, cPath, kPath, nullptr);
    }
}

bool HiveMqttClient::publish(const std::string& topic, const std::string& payload, int qos, bool retain) {
    if (!mosq_) return false;
    int rc = mosquitto_publish(mosq_, nullptr, topic.c_str(), 
                              utils::cast<int>(payload.length()), payload.c_str(), 
                              qos, retain);
    return (rc == MOSQ_ERR_SUCCESS);
}

bool HiveMqttClient::subscribe(const std::string& topic, int qos) {
    if (!mosq_) return false;
    int rc = mosquitto_subscribe(mosq_, nullptr, topic.c_str(), qos);
    return (rc == MOSQ_ERR_SUCCESS);
}

void HiveMqttClient::setOnMessageCallback(MessageCallback cb) {
    on_msg_cb_ = cb;
}

void HiveMqttClient::setOnConnectCallback(std::function<void(bool)> cb) {
    on_connect_cb_ = cb;
}

void HiveMqttClient::loop(int timeout_ms) {
    if (mosq_) mosquitto_loop(mosq_, timeout_ms, 1);
}

// Callbacks
void HiveMqttClient::on_connect_wrapper(struct mosquitto *mosq, void *obj, int rc) {
    auto* self = utils::as<HiveMqttClient>(obj);
    {
        std::lock_guard<std::mutex> lock(self->mtx_);
        self->connected_ = (rc == 0);
    }
    if (self->on_connect_cb_) self->on_connect_cb_(rc == 0);
    if (rc == 0) LOG_INFO("HiveMQ: Connected successfully");
}

void HiveMqttClient::on_message_wrapper(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    auto* self = utils::as<HiveMqttClient>(obj);
    if (self->on_msg_cb_ && msg->payload) {
        MqttMessage message;
        message.topic = msg->topic;
        message.payload = std::string(utils::as<const char>(msg->payload), msg->payloadlen);
        message.qos = msg->qos;
        message.retain = msg->retain;
        self->on_msg_cb_(message);
    }
}

} // namespace iot
