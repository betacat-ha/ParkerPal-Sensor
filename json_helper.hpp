#ifndef JSON_HELPER_HPP
#define JSON_HELPER_HPP

#include <ArduinoJson.h>

constexpr int MAX_NETWORKS = 30;

struct ParkingSpaceStatus {
    String spaceName;
    int occupyStatus;
    int reservationStatus;
    float distance;
};

struct NetworkInfo {
    String ssid;                          // WiFi网络名称
    int rssi;                             // WiFi信号强度
};

struct WiFiScanList {
    NetworkInfo networks[MAX_NETWORKS];
    int count;                            // 实际扫描到的网络数量
};

String fromJsonStruct(const ParkingSpaceStatus& parkingSpaceStatus);

String fromJsonStruct(const WiFiScanList& wiFiScanList);

JsonObject getJsonObject(JsonDocument& doc, const ParkingSpaceStatus& parkingSpaceStatus);

JsonObject getJsonObject(JsonDocument& doc, const WiFiScanList& wiFiScanList);

JsonObject stringToJsonObject(JsonDocument& doc, const String& jsonString);


#endif // JSON_HELPER_HPP