#include "wifi_handler.hpp"

WiFiHandler::WiFiHandler(const char* ssid, const char* password) 
    : _ssid(ssid), _password(password) {} // 初始化

bool WiFiHandler::connect() {
    // 设置WiFi模式为Station模式
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    // 使用格式化字符串构建日志信息
    char buffer[64]; // 假设最大缓冲区大小为64字节
    snprintf(buffer, sizeof(buffer), "[Wi-Fi] 连接到 %s", _ssid);
    Log.noticeln("%s", buffer);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(1000);
        snprintf(buffer, sizeof(buffer), "[Wi-Fi] 等待连接%ds，状态：%s", retry++, localizableWLStatus(WiFi.status()).c_str());
        Log.verboseln("%s", buffer);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Log.noticeln("[Wi-Fi] 已连接！");

        // 打印IP地址
        snprintf(buffer, sizeof(buffer), "[Wi-Fi] IP地址：%s", WiFi.localIP().toString().c_str());
        Log.noticeln("%s", buffer);
        return true;
    } else {
        Log.errorln("[Wi-Fi] 无法连接至Wi-Fi.");
        return false;
    }
}

int WiFiHandler::getRSSI() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    } else {
        Log.error("[Wi-Fi] 未连接。");
        return -1; // 或者其他表示未连接的值
    }
}

void WiFiHandler::scanAndPrintNetworks() {
    Log.noticeln("[Wi-Fi] 扫描AP...");

    int numberOfNetworks = WiFi.scanNetworks(); // 扫描可用的Wi-Fi网络

    if (numberOfNetworks == 0) {
        Log.noticeln("[Wi-Fi] 未找到AP。");
    } else {
        char buffer[64]; // 假设最大缓冲区大小为64字节

        // 打印找到的网络数量
        snprintf(buffer, sizeof(buffer), "[Wi-Fi] 找到%d个AP", numberOfNetworks);
        Log.noticeln("%s", buffer);

        for (int i = 0; i < numberOfNetworks; ++i) {
            // 打印网络编号、SSID 和信号强度
            snprintf(buffer, sizeof(buffer), "[Wi-Fi] %d: %s (%d dBm)", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            Log.notice("%s - %s", buffer, WiFi.encryptionType(i) == ENC_TYPE_NONE ? "开放" : "加密");
        }
    }

    // 扫描完成后，WiFi扫描列表会被清除，以释放内存
    WiFi.scanDelete();
}