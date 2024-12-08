#include "mqtt_handler.hpp"

// 内部管理的WiFiClient实例
static WiFiClient espClient;

WiFiClient& MQTTHandler::getWiFiClient() {
    return espClient;
}

MQTTHandler::MQTTHandler(const char* serverAddress, uint16_t port,
                         const char* username, const char* password)
    : _serverAddress(serverAddress), _port(port),
      _username(username), _password(password) {
    _mqttClient.setClient(getWiFiClient());
    _mqttClient.setServer(_serverAddress, _port);
}

void MQTTHandler::setCallback(void (*callback)(char*, byte*, unsigned int)) {
    _mqttClient.setCallback(callback);
}

bool MQTTHandler::connect() {
    String clientId = "esp8266-" + WiFi.macAddress();
    Log.noticeln("Starting MQTT connection...");
    Log.noticeln("Client ID: %s", clientId.c_str());

    if (WiFi.status() != WL_CONNECTED) {
        Log.errorln("Wi-Fi not connected.");
        return false;
    }

    while (!_mqttClient.connect(clientId.c_str(), _username, _password)) {
        Log.noticeln("Waiting for MQTT connection...");
        delay(1000);
    }
    Log.noticeln("MQTT connected!");
    return true;
}

void MQTTHandler::subscribeTopic(const char* topic) {
    if (_mqttClient.subscribe(topic)) {
        Log.noticeln("Subscribed to topic: %s", topic);
    } else {
        Log.errorln("Failed to subscribe to topic: %s", topic);
    }
}

void MQTTHandler::publishMessage(const char* topic, const char* message) {
    if (_mqttClient.publish(topic, message)) {
        Log.noticeln("Published message to topic %s: %s", topic, message);
    } else {
        Log.errorln("Failed to publish message to topic %s", topic);
    }
}

void MQTTHandler::loop() {
    _mqttClient.loop();
}