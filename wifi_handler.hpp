#ifndef WIFI_HANDLER_HPP
#define WIFI_HANDLER_HPP

#include <ESP8266WiFi.h>
#include "log_helper.hpp"
#include "json_helper.hpp"

typedef bool (*FilterFunction)(const NetworkInfo&);

class WiFiHandler {
public:
    WiFiHandler(const char* ssid, const char* password);
    bool connect();
    int getRSSI();                // 获取当前连接WiFi的RSSI
    void scanAndPrintNetworks();  // 扫描并打印可见WiFi网络信息
    WiFiScanList scanNetworks();  // 扫描并返回可见WiFi网络信息
    WiFiScanList scanNetworks(FilterFunction filter);
private:
    const char* _ssid;
    const char* _password;
};

#endif // WIFI_HANDLER_HPP