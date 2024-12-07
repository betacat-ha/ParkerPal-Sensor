#include "wifi_handler.hpp"

WiFiHandler::WiFiHandler(const char* ssid, const char* password) 
    : _ssid(ssid), _password(password) {} // 初始化

bool WiFiHandler::connect() {
    // 设置WiFi模式为Station模式
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    // 使用格式化字符串构建日志信息
    char buffer[64]; // 假设最大缓冲区大小为64字节
    snprintf(buffer, sizeof(buffer), "Connecting to %s", _ssid);
    Log.noticeln("%s", buffer);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(1000);
        snprintf(buffer, sizeof(buffer), "Waiting for %ds...", retry++);
        Log.noticeln("%s", buffer);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Log.noticeln("WiFi connected!");

        // 打印IP地址
        snprintf(buffer, sizeof(buffer), "IP address: %s", WiFi.localIP().toString());
        Log.noticeln("%s", buffer);
        return true;
    } else {
        Log.error("Failed to connect to WiFi.");
        return false;
    }
}

int WiFiHandler::getRSSI() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    } else {
        Log.error("Not connected to WiFi.");
        return -1; // 或者其他表示未连接的值
    }
}

void WiFiHandler::scanAndPrintNetworks() {
    Log.noticeln("Scanning WiFi networks...");

    int numberOfNetworks = WiFi.scanNetworks(); // 扫描可用的Wi-Fi网络

    if (numberOfNetworks == 0) {
        Log.noticeln("No Wi-Fi networks found.");
    } else {
        char buffer[64]; // 假设最大缓冲区大小为64字节

        // 打印找到的网络数量
        snprintf(buffer, sizeof(buffer), "%d networks found", numberOfNetworks);
        Log.noticeln("%s", buffer);

        for (int i = 0; i < numberOfNetworks; ++i) {
            // 打印网络编号、SSID 和信号强度
            snprintf(buffer, sizeof(buffer), "%d: %s (%d dBm)", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            Log.noticeln("%s", buffer);

            // 检查网络是否加密
            if (WiFi.encryptionType(i) == ENC_TYPE_NONE) {
                Log.noticeln(" - Open");
            } else {
                Log.noticeln(" - Secure");
            }
        }
    }

    // 扫描完成后，WiFi扫描列表会被清除，以释放内存
    WiFi.scanDelete();
}