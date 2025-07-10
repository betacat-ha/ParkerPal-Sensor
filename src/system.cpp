/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Original Copyright(C) 2024 by ParkerPal
 */

#include "system.hpp"

Preferences preferences;

/**
 * 保存系统设置
 * @param settings 系统设置
 */
void saveConfig(const SystemSettings &settings) {
    saveDeviceConfig(settings.deviceSettings);
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
 * 保存MQTT设置
 * @param settings MQTT设置
 */
void saveMQTTConfig(const MQTTSettings &settings) {
    preferences.begin("mqtt", false);
    preferences.putString("serverIP", settings.serverIP);
    preferences.putInt("serverPort", settings.serverPort);
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
 * 加载MQTT设置
 * @param settings MQTT设置
 */
void loadMQTTConfig(MQTTSettings &settings) {
    preferences.begin("mqtt", true);
    settings.serverIP = preferences.getString("serverIP", "");
    settings.serverPort = preferences.getInt("serverPort", 0);
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
    Log.verboseln("[System] 设备状态：%d", settings.deviceStatus);
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
 * 设置系统为未配置
 */
void setDeviceUnConfigured() {
    DeviceSettings settings;
    loadDeviceConfig(settings);
    settings.deviceStatus = 0;
    saveDeviceConfig(settings);
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
    deviceSettings.deviceLocation = doc["loc"].as<String>();
    saveDeviceConfig(deviceSettings);

    // 保存停车位配置
    ParkingSpaceList parkingSpaceList;
    parkingSpaceList.count = doc["ps"].size();
    for (int i = 0; i < parkingSpaceList.count; ++i) {
        parkingSpaceList.spaces[i].id = doc["ps"][i]["id"].as<String>();
        parkingSpaceList.spaces[i].spaceName = doc["ps"][i]["name"].as<String>();
        parkingSpaceList.spaces[i].slot = doc["ps"][i]["slot"].as<int>();
    }
    saveParkingSpaceConfig(parkingSpaceList);

    // 保存MQTT配置
    MQTTSettings mqttSettings;
    loadMQTTConfig(mqttSettings);
    mqttSettings.serverIP = doc["mq"]["ip"].as<String>();
    mqttSettings.serverPort = doc["mq"]["port"].as<int>();
    mqttSettings.serverUser = doc["mq"]["usr"].as<String>();
    mqttSettings.serverPassword = doc["mq"]["pwd"].as<String>();
    saveMQTTConfig(mqttSettings);

    setDeviceConfigured();

    Log.verboseln("[System] 保存服务器配置成功。");
}

/**
 * 擦除所有配置
 */
void eraseAllConfig() {
    preferences.begin("device", false);
    preferences.clear();
    preferences.end();

    preferences.begin("mqtt", false);
    preferences.clear();
    preferences.end();

    preferences.begin("parkingSpace", false);
    preferences.clear();
    preferences.end();

#ifdef ESP8266
    ESP.eraseConfig();
#elif defined(ESP32)
    nvs_flash_erase();           // 擦除整个 NVS 分区
    nvs_flash_init();            // 重新初始化
#endif
}

/**
 * 同步系统时间
 */
void syncSystemTime() {
    configTime(8 * 3600, 0, "ntp1.aliyun.com", "ntp.ntsc.ac.cn", "cn.ntp.org.cn");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        now = time(nullptr);
    }
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char buffer[64]; // 假设最大缓冲区大小为64字节
    snprintf(buffer, sizeof(buffer), "[System] 当前时间：%d-%02d-%02d %02d:%02d:%02d",
        timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Log.noticeln("%s", buffer);
}