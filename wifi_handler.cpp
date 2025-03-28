#include "wifi_handler.hpp"

WiFiHandler::WiFiHandler(const char* ssid, const char* password) 
    : _ssid(ssid), _password(password) {} // 初始化

bool WiFiHandler::smartConfig() {
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();

    Log.noticeln("[Wi-Fi] 启动智能配网...");

    while (!WiFi.smartConfigDone()) {
        delay(500);
        Log.verboseln("[Wi-Fi] 等待智能配网...");
    }

    Log.noticeln("[Wi-Fi] 智能配网完成！");

    // 连接到Wi-Fi
    return connect(false);
}

/**
 * @brief 连接到Wi-Fi
 * @return 是否连接成功
 */
bool WiFiHandler::connect() {
    return connect(false);
}

/**
 * @brief 连接到Wi-Fi
 * @param overrideSmartConfig 是否覆盖智能配网，值为true则使用该类初始化时的SSID和密码
 * @return 是否连接成功
 */
bool WiFiHandler::connect(bool overrideSmartConfig) {
    // 设置WiFi模式为Station模式
    WiFi.mode(WIFI_STA);
    if (overrideSmartConfig) {
        WiFi.begin(_ssid, _password);
    } else {
        WiFi.begin();
    }
    
    // 使用格式化字符串构建日志信息
    char buffer[64]; // 假设最大缓冲区大小为64字节
    snprintf(buffer, sizeof(buffer), "[Wi-Fi] 连接到 %s, 密码 %s", WiFi.SSID().c_str(), WiFi.psk().c_str());
    Log.noticeln("%s", buffer);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(1000);
        snprintf(buffer, sizeof(buffer), "[Wi-Fi] 等待连接%ds，状态：%s", retry++, localizableWLStatus(WiFi.status()).c_str());
        Log.verboseln("%s", buffer);
    }

    bool connected = WiFi.status() == WL_CONNECTED;

    if (!connected && overrideSmartConfig) {
        Log.errorln("[Wi-Fi] 无法连接至Wi-Fi.");
        return false;
    }

    if (!connected && !overrideSmartConfig) {
        if (_configRetryTimes <= 0) {
            Log.errorln("[Wi-Fi] 多次配网仍无法连接，请检查Wi-Fi配置！");
            return false;
        }
        
        Log.errorln("[Wi-Fi] 无法连接至Wi-Fi，启动智能配网...");
        _configRetryTimes--;
        return smartConfig();
    }

    Log.noticeln("[Wi-Fi] 已连接！");

    // 打印IP地址
    snprintf(buffer, sizeof(buffer), "[Wi-Fi] IP地址：%s", WiFi.localIP().toString().c_str());
    Log.noticeln("%s", buffer);
    return true;
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

    int numberOfNetworks = WiFi.scanNetworks();

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


WiFiScanList WiFiHandler::scanNetworks() {
    Log.noticeln("[Wi-Fi] 扫描AP...");

    WiFiScanList list;
    int numberOfNetworks = WiFi.scanNetworks();

    if (numberOfNetworks == 0) {
        Log.noticeln("[Wi-Fi] 未找到AP。");
        list.count = 0;
        return list;
    } 
    
    int i = 0;

    while(i < min(numberOfNetworks, MAX_NETWORKS)) {
        list.networks[i].ssid = WiFi.SSID(i);
        list.networks[i].rssi = WiFi.RSSI(i);
        ++i;
    }

    list.count = i;
    WiFi.scanDelete();
    Log.noticeln("[Wi-Fi] 扫描完成。");
    return list;
}

WiFiScanList WiFiHandler::scanNetworks(FilterFunction filter) {
  Log.noticeln("[Wi-Fi] 扫描AP（带过滤器）...");

    WiFiScanList list;
    int numberOfNetworks = WiFi.scanNetworks();

    if (numberOfNetworks == 0) {
        Log.noticeln("[Wi-Fi] 未找到AP（带过滤器）。");
        list.count = 0;
        return list;
    } 

    int i = 0, j = 0;
    while(i < numberOfNetworks) {
        NetworkInfo network = { WiFi.SSID(i), WiFi.RSSI(i) };
        if(filter(network)) {
          list.networks[j] = network;
          ++j;
        }
        if(j >= MAX_NETWORKS) break;
    }

    list.count = j;
    WiFi.scanDelete();
    Log.noticeln("[Wi-Fi] 扫描完成（带过滤器）。");
    return list;
}

/**
 * 处理抓包数据回调
 */
void ICACHE_FLASH_ATTR WiFiHandler::sniffer_callback(uint8_t* buffer,
    uint16_t length) {
    Serial.print("[Wi-Fi] 嗅探回调");
    struct SnifferPacket* snifferPacket = (struct SnifferPacket*)buffer;
    showMetadata(snifferPacket);
}


void WiFiHandler::printDataSpan(uint16_t start, uint16_t size, uint8_t* data) {
    for (uint16_t i = start; i < DATA_LENGTH && i < start + size; i++) {
        Serial.write(data[i]);
    }
}

/**
 * 获取嗅探包的MAC地址
 */
void WiFiHandler::getMAC(char* addr, uint8_t* data, uint16_t offset) {
    sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x", data[offset + 0],
        data[offset + 1], data[offset + 2], data[offset + 3],
        data[offset + 4], data[offset + 5]);
}

/**
 * 信道切换函数
 */
void WiFiHandler::channelHop() {
    // hoping channels 1-13
    uint8 new_channel = wifi_get_channel() + 1;
    if (new_channel > 13) {
        new_channel = 1;
    }

    Serial.print("[Wi-Fi] 切换频道至：");
    Serial.println(new_channel);

    wifi_set_channel(new_channel);
}

/**
 * 启动Wi-Fi嗅探器
 */
void WiFiHandler::startSniffer(wifi_promiscuous_cb_t cb) {
    // 设置Wi-Fi模式为监听模式
    wifi_set_opmode(STATION_MODE);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(cb);
    wifi_promiscuous_enable(1);
}