#pragma once

#include <stdint.h>

namespace Sniffer {

    struct PacketInfo {
        char mac[18];   // 设备MAC地址
        int8_t rssi;    // 信号强度
        char ssid[33];  // SSID名称
    };
    

    typedef void (*SnifferCallback)(const PacketInfo& packet);

    void begin(SnifferCallback cb);
    void setChannel(uint8_t channel);
    void end();

}
