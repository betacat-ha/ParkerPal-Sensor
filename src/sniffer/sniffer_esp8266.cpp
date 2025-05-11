// sniffer_esp8266.cpp
#ifdef ESP8266

#include "sniffer.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

extern "C" {
#include "user_interface.h"
}

#define DATA_LENGTH 112
#define TYPE_MANAGEMENT 0x00
#define SUBTYPE_PROBE_REQUEST 0x04
#define CHANNEL_HOP_INTERVAL_MS 1000

struct RxControl {
    signed rssi : 8;
    unsigned rate : 4;
    unsigned is_group : 1;
    unsigned : 1;
    unsigned sig_mode : 2;
    unsigned legacy_length : 12;
    unsigned damatch0 : 1;
    unsigned damatch1 : 1;
    unsigned bssidmatch0 : 1;
    unsigned bssidmatch1 : 1;
    unsigned MCS : 7;
    unsigned CWB : 1;
    unsigned HT_length : 16;
    unsigned Smoothing : 1;
    unsigned Not_Sounding : 1;
    unsigned : 1;
    unsigned Aggregation : 1;
    unsigned STBC : 2;
    unsigned FEC_CODING : 1;
    unsigned SGI : 1;
    unsigned rxend_state : 8;
    unsigned ampdu_cnt : 8;
    unsigned channel : 4;
    unsigned : 12;
};

struct SnifferPacket {
    struct RxControl rx_ctrl;
    uint8_t data[DATA_LENGTH];
    uint16_t cnt;
    uint16_t len;
};


namespace Sniffer {

    static SnifferCallback userCallback = nullptr;
    static os_timer_t channelHop_timer;

    static void getMAC(char* addr, uint8_t* data, uint16_t offset) {
        sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x",
            data[offset + 0], data[offset + 1], data[offset + 2],
            data[offset + 3], data[offset + 4], data[offset + 5]);
    }

    static void channelHop() {
        uint8_t new_channel = wifi_get_channel() + 1;
        if (new_channel > 13) new_channel = 1;
        wifi_set_channel(new_channel);
    }

    static void ICACHE_FLASH_ATTR sniffer_callback(uint8_t* buffer, uint16_t length) {
        struct SnifferPacket* packet = (struct SnifferPacket*)buffer;

        uint16_t frameControl = (packet->data[1] << 8) | packet->data[0];
        uint8_t frameType = (frameControl & 0x0C) >> 2;
        uint8_t frameSubType = (frameControl & 0xF0) >> 4;

        if (frameType == TYPE_MANAGEMENT && frameSubType == SUBTYPE_PROBE_REQUEST) {
            if (userCallback) {
                PacketInfo info;
                getMAC(info.mac, packet->data, 10);
                info.rssi = packet->rx_ctrl.rssi;

                uint8_t ssid_len = packet->data[25];
                if (ssid_len > 0 && ssid_len < 32) {
                    memcpy(info.ssid, &packet->data[26], ssid_len);
                    info.ssid[ssid_len] = '\0';
                } else {
                    info.ssid[0] = '\0';
                }

                userCallback(info);
            }
        }
    }

    void begin(SnifferCallback cb) {
        userCallback = cb;

        WiFi.mode(WIFI_STA);
        wifi_set_channel(1);
        wifi_promiscuous_enable(0);
        wifi_set_promiscuous_rx_cb(sniffer_callback);
        wifi_promiscuous_enable(1);

        os_timer_disarm(&channelHop_timer);
        os_timer_setfn(&channelHop_timer, (os_timer_func_t*)channelHop, NULL);
        os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL_MS, 1);
    }

    void setChannel(uint8_t channel) {
        wifi_set_channel(channel);
    }

    void end() {
        wifi_promiscuous_enable(0);
        os_timer_disarm(&channelHop_timer);
    }

}

#endif
