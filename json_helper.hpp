#ifndef JSON_HELPER_HPP
#define JSON_HELPER_HPP

#include <ArduinoJson.h>

constexpr int MAX_NETWORKS = 30;
constexpr int MAX_SPACE = 10;

struct ParkingSpaceStatus {
    String id;
    String spaceName;
    int occupyStatus;
    int reservationStatus;
    float distance;
};

struct SpaceStatusList {
    ParkingSpaceStatus spaces[MAX_SPACE];
    int count;                            // 实际设置的位置数量
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

String fromJsonStruct(const SpaceStatusList& spaceStatusList);

String fromJsonStruct(const WiFiScanList& wiFiScanList);

JsonObject getJsonObject(JsonDocument& doc, const ParkingSpaceStatus& parkingSpaceStatus);

JsonObject getJsonObject(JsonDocument& doc, const WiFiScanList& wiFiScanList);

JsonObject stringToJsonObject(JsonDocument& doc, const String& jsonString);


#endif // JSON_HELPER_HPP