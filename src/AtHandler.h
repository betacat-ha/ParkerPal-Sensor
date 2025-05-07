#pragma once

#include <Arduino.h>
#include <ArduinoLog.h>
#include <functional>
#include <map>

class AtHandler {
public:
    using Callback = std::function<void()>;
    using MQTTCallback = std::function<void(const char*, const char*)>;

    AtHandler(HardwareSerial& serial);

    void begin(unsigned long baud);
    void loop();

    void onOK(Callback cb);
    void onMQTTMessage(MQTTCallback cb);
    void onCustom(const String& keyword, Callback cb);

    int connectWiFi();
    int connectWiFi(const String& ssid, const String& password);
    int connectWiFi(const String& ssid, const String& password, bool overrideSmartConfig);
    int smartConfig(int retryTimes = 3);
    int startSmartConfig();
    int stopSmartConfig();
    int mqttConnect(const String& broker, int port, const String& clientId, const String& username, const String& password);
    int mqttSubscribe(const String& topic);
    int mqttPublish(const String& topic, const String& message);
    int mqttPublishWithRaw(const String& message, unsigned long timeout = 10000);
    int mqttPublishWithRaw(const uint8_t* data, size_t length, unsigned long timeout = 10000);
    int mqttPublishWithRaw(const String& topic, const uint8_t* data, size_t length, unsigned long timeout = 10000);
    int getWiFiStatus(String& status, String& ssid);
    int setWifiMode(int mode);
    int getMacAddress(String& mac);

    int enableEcho();
    int disableEcho();
    int enableSysLog();
    int disableSysLog();
    int restart();
    int setSNTPConfig();
    int syncTime();

    int setMqttSubTopic(const char* topic);
    int setMqttPubTopic(const char* topic);
    int setMqttClientId(const char* clientId);

private:
    HardwareSerial& espSerial;
    String buffer;

    Callback okCallback = nullptr;
    MQTTCallback mqttCallback = nullptr;
    std::map<String, Callback> customCallbacks;

    char* _mqttSubTopic;
    char* _mqttPubTopic;
    char* _mqttClientId;

    void handleMQTTMessage(const String& line);
    void processLine(const String& line);
    int sendATCommand(const String& command, const bool waitForResponse = true, const String& expectedResponse = "OK", unsigned long timeout = 10000);
    int sendATCommandWithPayload(const String& command, const uint8_t* payload, size_t length, const String& successResponse, const String& failureResponse, unsigned long timeout = 10000);
    int waitForResponse(const String& expectedKeyword, String& rawResponse, unsigned long timeout = 10000);
    int setString(const char* str, char** dest, char* operation);
};