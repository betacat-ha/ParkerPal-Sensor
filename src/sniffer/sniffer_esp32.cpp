#ifdef ESP32

#include "sniffer.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define CHANNEL_HOP_INTERVAL_MS 300

namespace Sniffer {

    static SnifferCallback userCallback = nullptr;
    static esp_timer_handle_t channelHopTimer;

    static void channelHop() {
        uint8_t new_channel;
        wifi_second_chan_t second_channel;
        esp_wifi_get_channel(&new_channel, &second_channel);
        new_channel++;
        if (new_channel > 13) new_channel = 1;
        esp_wifi_set_channel(new_channel, WIFI_SECOND_CHAN_NONE);
    }

    // 定时器回调函数
    static void channelHopTimerCb(void* arg) {
        channelHop();
    }

    static void wifi_sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
        const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
        const uint8_t* data = pkt->payload;

        uint16_t frameCtrl = (data[1] << 8) | data[0];
        uint8_t frameType = (frameCtrl & 0x0C) >> 2;
        uint8_t frameSubType = (frameCtrl & 0xF0) >> 4;

        if (frameType == 0x00 && frameSubType == 0x04) { // Probe Request
            if (userCallback) {
                char mac[18];
                sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
                    data[10], data[11], data[12],
                    data[13], data[14], data[15]);

                uint8_t ssid_len = data[25];
                char ssid[33] = { 0 };
                if (ssid_len > 0 && ssid_len < 32) {
                    memcpy(ssid, &data[26], ssid_len);
                    ssid[ssid_len] = '\0';
                }

                PacketInfo info;
                strcpy(info.mac, mac);
                info.rssi = pkt->rx_ctrl.rssi;
                strncpy(info.ssid, ssid, sizeof(info.ssid) - 1);
                info.ssid[sizeof(info.ssid) - 1] = '\0';

                userCallback(info);
            }
        }
    }

    void begin(SnifferCallback cb) {
        userCallback = cb;

        WiFi.mode(WIFI_MODE_STA);
        esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
        esp_wifi_set_promiscuous(false);
        esp_wifi_set_promiscuous_rx_cb(wifi_sniffer_callback);
        esp_wifi_set_promiscuous(true);

        // 创建定时器用于信道切换
        const esp_timer_create_args_t timerArgs = {
            .callback = &channelHopTimerCb,
            .name = "ChannelHopTimer"
        };
        esp_timer_create(&timerArgs, &channelHopTimer);
        esp_timer_start_periodic(channelHopTimer, CHANNEL_HOP_INTERVAL_MS * 1000); // 每300ms切换一次信道
    }

    void setChannel(uint8_t channel) {
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    }

    void end() {
        esp_wifi_set_promiscuous(false);
    }

}

#endif
