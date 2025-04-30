#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "cross_platform.hpp"
#include "log_helper.hpp"
#include <ArduinoJson.h>
#include <Preferences.h> // ESP8266 by Volodymyr Shymanskyy
#include INTERNAL_STORAGE_LIBRARY

constexpr int SLOT_AMOUNT = 4;

// 设备设置
struct DeviceSettings {
    String deviceName;
    String deviceType;
    String deviceLocation;
    String deviceOwner;
    String deviceModel;
    String deviceVersion;
    String upgradeURL;
    String deviceIP;
    String deviceMAC;
    String deviceToken;
    int deviceStatus; // 设备状态，1表示初始化完成，0表示未初始化，-1表示状态未知
};

// MQTT设置
struct MQTTSettings {
    String serverIP;
    int serverPort;
    String serverUser;
    String serverPassword;
};

// 车位传感器配置
struct ParkingSpace {
    String id;
    String spaceName;
    int slot;
};

// 车位传感器配置列表
struct ParkingSpaceList {
    ParkingSpace spaces[SLOT_AMOUNT];
    int count;
};

// 系统设置
struct SystemSettings {
    DeviceSettings deviceSettings;
    MQTTSettings mqttSettings;
    ParkingSpaceList parkingSpaceList;
};

void saveConfig(const SystemSettings &settings);
void saveDeviceConfig(const DeviceSettings &settings);
void saveMQTTConfig(const MQTTSettings &settings);
void saveParkingSpaceConfig(const ParkingSpaceList &settings);
void loadConfig(SystemSettings &settings);
void loadDeviceConfig(DeviceSettings &settings);
void loadMQTTConfig(MQTTSettings &settings);
void loadParkingSpaceConfig(ParkingSpaceList &settings);
bool isDeviceConfigured();
void setDeviceConfigured();
void saveServerConfig(const char *json);
void saveServerConfig(const JsonObject &doc);
void eraseAllConfig();
void syncSystemTime();
#endif // SYSTEM_HPP