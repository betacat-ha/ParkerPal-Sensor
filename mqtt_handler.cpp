#include "mqtt_handler.hpp"

// 内部管理的WiFiClient实例
static WiFiClient espClient;

WiFiClient &MQTTHandler::getWiFiClient() { return espClient; }

MQTTHandler::MQTTHandler(const char *serverAddress, uint16_t port,
                         const char *username, const char *password, 
                         const char *clientId, const char *subTopic, const char *pubTopic)
    : _serverAddress(serverAddress), _port(port), _username(username),
      _password(password), _clientId(clientId), _subTopic(subTopic), 
      _pubTopic(pubTopic) {
    _mqttClient.setClient(getWiFiClient());
    _mqttClient.setServer(_serverAddress, _port);
}

void MQTTHandler::setCallback(void (*callback)(char *, byte *, unsigned int)) {
    _mqttClient.setCallback(callback);
}

void MQTTHandler::setBufferSize(int size) {
    _mqttClient.setBufferSize(size);
}


bool MQTTHandler::connect() {
    Log.noticeln("[MQTT] 连接到：%s", _serverAddress);
    Log.noticeln("[MQTT] Client ID: %s", _clientId);

    if (WiFi.status() != WL_CONNECTED) {
        Log.errorln("[MQTT] Wi-Fi 未连接");
        return false;
    }

    while (!_mqttClient.connect(_clientId, _username, _password)) {
        Log.noticeln("[MQTT] 等待与服务器建立连接...");
        delay(1000);
    }
    Log.noticeln("[MQTT] 服务器连接成功！");
    return true;
}

/**
* 订阅指定主题
* @param topic 主题名
*/
void MQTTHandler::subscribeTopic(const char *topic) {
    if (_mqttClient.subscribe(topic)) {
        Log.noticeln("[MQTT] 订阅主题: %s", topic);
    } else {
        Log.errorln("[MQTT] 无法订阅主题: %s", topic);
    }
}

/**
* 订阅默认主题
*/
void MQTTHandler::subscribeTopic() {
    subscribeTopic(_subTopic);
}

void MQTTHandler::publishMessage(const char *topic, const char *message) {
    if (_mqttClient.publish(topic, message)) {
        Log.noticeln("[MQTT] 发布消息到主题[%s]: %s", topic, message);
    } else {
        Log.errorln("[MQTT] 无法发布消息到主题[%s]: %s", topic, message);
    }
}

/**
* 使用默认主题发送信息
*/
void MQTTHandler::publishMessage(const char *message) {
    publishMessage(_pubTopic, message);
}

void MQTTHandler::loop() {
    _mqttClient.loop();
}

