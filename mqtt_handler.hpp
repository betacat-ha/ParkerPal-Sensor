#ifndef MQTT_HANDLER_HPP
#define MQTT_HANDLER_HPP

#include "cross_platform.hpp"
#include <PubSubClient.h>
#include WIFI_LIBRARY
#include "log_helper.hpp"
#include "operations.hpp"

class MQTTHandler {
public:
    MQTTHandler(const char* serverAddress, uint16_t port,
                const char* username = "", const char* password = "");
    void setCallback(void (*callback)(char*, byte*, unsigned int));
    void setBufferSize(int size);
    bool connect();
    void subscribeTopic();
    void subscribeTopic(const char* topic);
    void publishMessage(const char* message);
    void publishMessage(const char* topic, const char* message);
    void loop();
    void callback(char* topic, byte* payload, unsigned int length);

private:
    PubSubClient _mqttClient;
    const char* _serverAddress;
    uint16_t _port;
    const char* _username;
    const char* _password;

    WiFiClient& getWiFiClient(); // 返回内部管理的WiFiClient实例
};

#endif // MQTT_HANDLER_HPP