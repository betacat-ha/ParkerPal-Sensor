#ifndef WIFI_HANDLER_HPP
#define WIFI_HANDLER_HPP

#include <ESP8266WiFi.h>
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

    void startSniffer(wifi_promiscuous_cb_t cb);          // 启动Wi-Fi嗅探器
    void stopSniffer();           // 停止Wi-Fi嗅探器

private:
    const char* _ssid;
    const char* _password;
    int _configRetryTimes = 3;    // 配置重试次数
    void ICACHE_FLASH_ATTR sniffer_callback(uint8_t* buffer, uint16_t length); // 嗅探回调函数
    void printDataSpan(uint16_t start, uint16_t size, uint8_t* data);
    void getMAC(char* addr, uint8_t* data, uint16_t offset); // 获取MAC地址
    void channelHop();
};

#endif // WIFI_HANDLER_HPP