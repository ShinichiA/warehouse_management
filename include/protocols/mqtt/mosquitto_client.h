#pragma once

#include "interface/i_mqtt_client.h"
#include <mosquitto.h>
#include <mutex>
#include <memory>

namespace iot {

/**
 * @brief Implementation of IMqttClient using Mosquitto.
 */
class MosquittoClient : public IMqttClient {
public:
    MosquittoClient(const std::string& id = "");
    ~MosquittoClient() override;

    // Disallow copying
    MosquittoClient(const MosquittoClient&) = delete;
    MosquittoClient& operator=(const MosquittoClient&) = delete;

    bool connect(const std::string& host, int port, int keepalive) override;
    void disconnect() override;
    bool isConnected() const override;

    void setCredentials(const std::string& username, const std::string& password) override;
    void setTls(const std::string& caPath, const std::string& certPath, const std::string& keyPath) override;

    bool publish(const std::string& topic, const std::string& payload, int qos, bool retain) override;
    bool subscribe(const std::string& topic, int qos) override;

    void setOnMessageCallback(MessageCallback cb) override;
    void setOnConnectCallback(std::function<void(bool success)> cb) override;

    void loop(int timeout_ms) override;

private:
    // Static callbacks required by C library libmosquitto
    static void on_connect_wrapper(struct mosquitto *mosq, void *obj, int rc);
    static void on_message_wrapper(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
    static void on_disconnect_wrapper(struct mosquitto *mosq, void *obj, int rc);

    struct mosquitto *mosq_ = nullptr;
    bool connected_ = false;
    bool loop_running_ = false;
    
    MessageCallback on_msg_cb_ = nullptr;
    std::function<void(bool)> on_connect_cb_ = nullptr;
    
    mutable std::mutex mtx_;
};

} // namespace iot
