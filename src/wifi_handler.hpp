/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Original Copyright(C) 2024 by ParkerPal
 */

#ifndef WIFI_HANDLER_HPP
#define WIFI_HANDLER_HPP

#include "log_helper.hpp"
#include "json_helper.hpp"

// 跨平台兼容
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

typedef bool (*FilterFunction)(const NetworkInfo&);

class WiFiHandler {
public:
    WiFiHandler(const char* ssid, const char* password);
    bool smartConfig();           // 启动智能配网
    bool connect();
    bool connect(bool overrideSmartConfig);
    int getRSSI();                // 获取当前连接WiFi的RSSI
    void scanAndPrintNetworks();  // 扫描并打印可见WiFi网络信息
    WiFiScanList scanNetworks();  // 扫描并返回可见WiFi网络信息
    WiFiScanList scanNetworks(FilterFunction filter);

private:
    const char* _ssid;
    const char* _password;
    int _configRetryTimes = 3;    // 配置重试次数
};

#endif // WIFI_HANDLER_HPP