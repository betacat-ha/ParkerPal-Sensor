#include "system.hpp"

Preferences preferences;

/**
 * 保存系统设置
 * @param settings 系统设置
 */
void saveConfig(const SystemSettings &settings) {
    saveDeviceConfig(settings.deviceSettings);
    saveAPConfig(settings.apSettings);
    saveMQTTConfig(settings.mqttSettings);
    saveParkingSpaceConfig(settings.parkingSpaceList);
}

/**
 * 保存设备设置
 * @param settings 设备设置
 */
void saveDeviceConfig(const DeviceSettings &settings) {
    preferences.begin("device", false);
    preferences.putString("deviceName", settings.deviceName);
    preferences.putString("deviceType", settings.deviceType);
    preferences.putString("deviceLocation", settings.deviceLocation);
    preferences.putString("deviceOwner", settings.deviceOwner);
    preferences.putString("deviceModel", settings.deviceModel);
    preferences.putString("deviceVersion", settings.deviceVersion);
    preferences.putString("upgradeURL", settings.upgradeURL);
    preferences.putString("deviceIP", settings.deviceIP);
    preferences.putString("deviceMAC", settings.deviceMAC);
    preferences.putString("deviceToken", settings.deviceToken);
    preferences.putInt("deviceStatus", settings.deviceStatus);
    preferences.end();
}

/**
 * 保存AP设置
 * @param settings AP设置
 */
void saveAPConfig(const APSettings &settings) {
    preferences.begin("ap", false);
    preferences.putString("SSID", settings.SSID);
    preferences.putString("Password", settings.Password);
    preferences.end();
}

/**
 * 保存MQTT设置
 * @param settings MQTT设置
 */
void saveMQTTConfig(const MQTTSettings &settings) {
    preferences.begin("mqtt", false);
    preferences.putString("serverIP", settings.serverIP);
    preferences.putString("serverPort", settings.serverPort);
    preferences.putString("serverUser", settings.serverUser);
    preferences.putString("serverPassword", settings.serverPassword);
    preferences.end();
}

/**
 * 保存车位传感器配置
 * @param settings 车位传感器配置
 */
void saveParkingSpaceConfig(const ParkingSpaceList &settings) {
    preferences.begin("parkingSpace", false);
    preferences.putInt("count", settings.count);
    for (int i = 0; i < settings.count; ++i) {
        preferences.putString(("id" + String(i)).c_str(), settings.spaces[i].id);
        preferences.putString(("spaceName" + String(i)).c_str(), settings.spaces[i].spaceName);
        preferences.putInt(("slot" + String(i)).c_str(), settings.spaces[i].slot);
    }
    preferences.end();
}

/**
 * 加载系统设置
 * @param settings 系统设置
 */
void loadConfig(SystemSettings &settings) {
    loadDeviceConfig(settings.deviceSettings);
    loadAPConfig(settings.apSettings);
    loadMQTTConfig(settings.mqttSettings);
    loadParkingSpaceConfig(settings.parkingSpaceList);
}

/**
 * 加载设备设置
 * @param settings 设备设置
 */
void loadDeviceConfig(DeviceSettings &settings) {
    preferences.begin("device", true);
    settings.deviceName = preferences.getString("deviceName", "");
    settings.deviceType = preferences.getString("deviceType", "");
    settings.deviceLocation = preferences.getString("deviceLocation", "");
    settings.deviceOwner = preferences.getString("deviceOwner", "");
    settings.deviceModel = preferences.getString("deviceModel", "");
    settings.deviceVersion = preferences.getString("deviceVersion", "");
    settings.upgradeURL = preferences.getString("upgradeURL", "");
    settings.deviceIP = preferences.getString("deviceIP", "");
    settings.deviceMAC = preferences.getString("deviceMAC", "");
    settings.deviceToken = preferences.getString("deviceToken", "");
    settings.deviceStatus = preferences.getInt("deviceStatus", 0);
    preferences.end();
}

/**
 * 加载AP设置
 * @param settings AP设置
 */
void loadAPConfig(APSettings &settings) {
    preferences.begin("ap", true);
    settings.SSID = preferences.getString("SSID", "");
    settings.Password = preferences.getString("Password", "");
    preferences.end();
}

/**
 * 加载MQTT设置
 * @param settings MQTT设置
 */
void loadMQTTConfig(MQTTSettings &settings) {
    preferences.begin("mqtt", true);
    settings.serverIP = preferences.getString("serverIP", "");
    settings.serverPort = preferences.getString("serverPort", "");
    settings.serverUser = preferences.getString("serverUser", "");
    settings.serverPassword = preferences.getString("serverPassword", "");
    preferences.end();
}

/**
 * 加载车位传感器配置
 * @param settings 车位传感器配置
 */
void loadParkingSpaceConfig(ParkingSpaceList &settings) {
    preferences.begin("parkingSpace", true);
    settings.count = preferences.getInt("count", 0);
    for (int i = 0; i < settings.count; ++i) {
        settings.spaces[i].id = preferences.getString(("id" + String(i)).c_str(), "");
        settings.spaces[i].spaceName = preferences.getString(("spaceName" + String(i)).c_str(), "");
        settings.spaces[i].slot = preferences.getInt(("slot" + String(i)).c_str(), 0);
    }
    preferences.end();
}

/**
 * 判断设备是否已配置
 * @return 是否已配置
 */
bool isDeviceConfigured() {
    DeviceSettings settings;
    loadDeviceConfig(settings);
    return settings.deviceStatus == 1;
}

/**
 * 设置设备已配置
 */
void setDeviceConfigured() {
    DeviceSettings settings;
    loadDeviceConfig(settings);
    settings.deviceStatus = 1;
    saveDeviceConfig(settings);
}

/**
 * 保存从服务器获取的配置
 * @param json 从服务器获取的配置-String
 */
void saveServerConfig(const char *json) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Log.errorln("[System] 服务器配置解析失败：%s", error.c_str());
        return;
    }

    // 保存设备配置
    DeviceSettings deviceSettings;
    loadDeviceConfig(deviceSettings);
    deviceSettings.deviceName = doc["name"].as<String>();
    deviceSettings.deviceType = doc["role"].as<String>();
    deviceSettings.deviceLocation = doc["location"].as<String>();
    deviceSettings.deviceStatus = 1;
    saveDeviceConfig(deviceSettings);

    // 保存停车位配置
    ParkingSpaceList parkingSpaceList;
    parkingSpaceList.count = doc["parkingSpaces"].size();
    for (int i = 0; i < parkingSpaceList.count; ++i) {
        parkingSpaceList.spaces[i].id = doc["parkingSpaces"][i]["id"].as<String>();
        parkingSpaceList.spaces[i].spaceName = doc["parkingSpaces"][i]["name"].as<String>();
        parkingSpaceList.spaces[i].slot = doc["parkingSpaces"][i]["slot"].as<int>();
    }
    saveParkingSpaceConfig(parkingSpaceList);
}

/**
 * 保存从服务器获取的配置
 * @param doc 从服务器获取的配置-JsonObject
 */
void saveServerConfig(const JsonObject &doc) {
    Log.noticeln("[System] 保存服务器配置。");
    // 保存设备配置
    DeviceSettings deviceSettings;
    loadDeviceConfig(deviceSettings);
    deviceSettings.deviceName = doc["name"].as<String>();
    deviceSettings.deviceType = doc["role"].as<String>();
    deviceSettings.deviceLocation = doc["location"].as<String>();
    deviceSettings.deviceStatus = 1;
    saveDeviceConfig(deviceSettings);

    // 保存停车位配置
    ParkingSpaceList parkingSpaceList;
    parkingSpaceList.count = doc["parkingSpaces"].size();
    for (int i = 0; i < parkingSpaceList.count; ++i) {
        parkingSpaceList.spaces[i].id = doc["parkingSpaces"][i]["id"].as<String>();
        parkingSpaceList.spaces[i].spaceName = doc["parkingSpaces"][i]["name"].as<String>();
        parkingSpaceList.spaces[i].slot = doc["parkingSpaces"][i]["slot"].as<int>();
    }
    saveParkingSpaceConfig(parkingSpaceList);
    Log.verboseln("[System] 保存服务器配置成功。");
}