#include "protocols/mqtt/mosquitto_client.h"
#include "utils/logger.h"
#include "utils/helper.h"
#include <cstring>

namespace iot {

MosquittoClient::MosquittoClient(const std::string& id) {
    mosquitto_lib_init();
    mosq_ = mosquitto_new(id.empty() ? nullptr : id.c_str(), true, this);

    if (mosq_) {
        mosquitto_connect_callback_set(mosq_, on_connect_wrapper);
        mosquitto_message_callback_set(mosq_, on_message_wrapper);
        mosquitto_disconnect_callback_set(mosq_, on_disconnect_wrapper);
    } else {
        LOG_ERROR("MQTT: Failed to create mosquitto instance");
    }
}

MosquittoClient::~MosquittoClient() {
    MosquittoClient::disconnect();
    if (mosq_) {
        mosquitto_destroy(mosq_);
        mosq_ = nullptr;
    }
    mosquitto_lib_cleanup();
}

bool MosquittoClient::connect(const std::string& host, int port, int keepalive) {
    if (!mosq_) return false;

    int rc = mosquitto_connect(mosq_, host.c_str(), port, keepalive);
    if (rc != MOSQ_ERR_SUCCESS) {
        LOG_ERROR("MQTT: Connect failed: " + std::string(mosquitto_strerror(rc)));
        return false;
    }

    // Start background thread for network traffic only once.
    if (!loop_running_) {
        rc = mosquitto_loop_start(mosq_);
        if (rc != MOSQ_ERR_SUCCESS) {
            LOG_ERROR("MQTT: loop_start failed: " + std::string(mosquitto_strerror(rc)));
            mosquitto_disconnect(mosq_);
            return false;
        }
        loop_running_ = true;
    }
    return true;
}

void MosquittoClient::disconnect() {
    if (mosq_) {
        mosquitto_disconnect(mosq_);
        if (loop_running_) {
            mosquitto_loop_stop(mosq_, true);
            loop_running_ = false;
        }
        connected_ = false;
    }
}

bool MosquittoClient::isConnected() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return connected_;
}

void MosquittoClient::setCredentials(const std::string& username, const std::string& password) {
    if (mosq_) {
        mosquitto_username_pw_set(mosq_, username.c_str(), password.c_str());
    }
}

void MosquittoClient::setTls(const std::string& caPath, const std::string& certPath, const std::string& keyPath) {
    if (mosq_) {
        const char* cert = certPath.empty() ? nullptr : certPath.c_str();
        const char* key = keyPath.empty() ? nullptr : keyPath.c_str();
        mosquitto_tls_set(mosq_, caPath.c_str(), nullptr, cert, key, nullptr);
    }
}

bool MosquittoClient::publish(const std::string& topic, const std::string& payload, int qos, bool retain) {
    if (!mosq_) return false;
    
    int rc = mosquitto_publish(mosq_, nullptr, topic.c_str(), 
                              utils::cast<int>(payload.length()), payload.c_str(), 
                              qos, retain);
    return (rc == MOSQ_ERR_SUCCESS);
}

bool MosquittoClient::subscribe(const std::string& topic, int qos) {
    if (!mosq_) return false;
    
    int rc = mosquitto_subscribe(mosq_, nullptr, topic.c_str(), qos);
    return (rc == MOSQ_ERR_SUCCESS);
}

void MosquittoClient::setOnMessageCallback(MessageCallback cb) {
    on_msg_cb_ = cb;
}

void MosquittoClient::setOnConnectCallback(std::function<void(bool)> cb) {
    on_connect_cb_ = cb;
}

void MosquittoClient::loop(int timeout_ms) {
    if (mosq_) {
        mosquitto_loop(mosq_, timeout_ms, 1);
    }
}

// Private Callbacks Implementation
void MosquittoClient::on_connect_wrapper(struct mosquitto *mosq, void *obj, int rc) {
    auto* self = utils::as<MosquittoClient>(obj);
    {
        std::lock_guard<std::mutex> lock(self->mtx_);
        self->connected_ = (rc == 0);
    }
    
    if (rc == 0) {
        LOG_INFO("MQTT: Connected successfully");
    } else {
        LOG_ERROR("MQTT: Connect wrapper returned error: " + std::to_string(rc));
    }
    
    if (self->on_connect_cb_) {
        self->on_connect_cb_(rc == 0);
    }
}

void MosquittoClient::on_message_wrapper(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    auto* self = utils::as<MosquittoClient>(obj);
    if (self->on_msg_cb_ && msg->payload) {
        MqttMessage message;
        message.topic = msg->topic;
        message.payload = std::string(utils::as<const char>(msg->payload), msg->payloadlen);
        message.qos = msg->qos;
        message.retain = msg->retain;
        
        self->on_msg_cb_(message);
    }
}

void MosquittoClient::on_disconnect_wrapper(struct mosquitto *mosq, void *obj, int rc) {
    auto* self = utils::as<MosquittoClient>(obj);
    std::lock_guard<std::mutex> lock(self->mtx_);
    self->connected_ = false;
    LOG_WARNING("MQTT: Disconnected from broker");
}

} // namespace iot
