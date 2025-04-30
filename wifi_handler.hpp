#ifndef WIFI_HANDLER_HPP
#define WIFI_HANDLER_HPP

#include "cross_platform.hpp"
#include WIFI_LIBRARY
#include "log_helper.hpp"
#include "json_helper.hpp"

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