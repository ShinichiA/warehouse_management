#pragma once

#include <string>
#include <functional>

namespace iot {

/**
 * @brief MQTT Message structure.
 */
struct MqttMessage {
    std::string topic;
    std::string payload;
    int qos;
    bool retain;
};

/**
 * @brief Interface for MQTT Client.
 */
class IMqttClient {
public:
    virtual ~IMqttClient() = default;

    /**
     * @brief Connect to the MQTT broker.
     * @param host Broker address (e.g., "localhost")
     * @param port Broker port (e.g., 1883)
     * @param keepalive Keepalive interval in seconds
     * @return true if connection attempt succeeded
     */
    virtual bool connect(const std::string& host, int port, int keepalive) = 0;

    /**
     * @brief Disconnect from the MQTT broker.
     */
    virtual void disconnect() = 0;

    /**
     * @brief Check if currently connected to the broker.
     */
    [[nodiscard]] virtual bool isConnected() const = 0;

    /**
     * @brief Set authentication credentials.
     */
    virtual void setCredentials(const std::string& username, const std::string& password) = 0;

    /**
     * @brief Configure TLS/SSL settings.
     */
    virtual void setTls(const std::string& caPath, const std::string& certPath , const std::string& keyPath) = 0;

    /**
     * @brief Publish a message to a topic.
     * @param topic Target topic
     * @param payload Message content
     * @param qos Quality of Service (0, 1, or 2)
     * @param retain Whether to retain the message on the broker
     * @return true if published successfully
     */
    virtual bool publish(const std::string& topic, const std::string& payload, int qos, bool retain) = 0;

    /**
     * @brief Subscribe to a topic.
     */
    virtual bool subscribe(const std::string& topic, int qos) = 0;

    // Callbacks
    using MessageCallback = std::function<void(const MqttMessage&)>;

    /**
     * @brief Set the callback function for incoming messages.
     */
    virtual void setOnMessageCallback(MessageCallback cb) = 0;
    
    /**
     * @brief Set the callback function for connection status changes.
     */
    virtual void setOnConnectCallback(std::function<void(bool success)> cb) = 0;

    /**
     * @brief Loop function to process network traffic.
     * Should be called frequently if not using a background thread.
     * @param timeout_ms Max time to wait for network activity
     */
    virtual void loop(int timeout_ms) = 0;
};

} // namespace iot
