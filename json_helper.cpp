#include "json_helper.hpp"

String fromJsonStruct(const ParkingSpaceStatus& parkingSpaceStatus) {
    StaticJsonDocument<256> doc;
    doc["spaceName"] = parkingSpaceStatus.spaceName.c_str();
    doc["occupyStatus"] = parkingSpaceStatus.occupyStatus;
    doc["reservationStatus"] = parkingSpaceStatus.reservationStatus;
    doc["distance"] = parkingSpaceStatus.distance;

    String output;
    serializeJson(doc, output);
    return output;
}

String fromJsonStruct(const WiFiScanList& wiFiScanList) {
    DynamicJsonDocument doc(1024);

    JsonArray networksArray = doc.createNestedArray("networks");

    for (int i = 0; i < wiFiScanList.count; ++i) {
        JsonObject networkObj = networksArray.createNestedObject();
        networkObj["ssid"] = wiFiScanList.networks[i].ssid.c_str();
        networkObj["rssi"] = wiFiScanList.networks[i].rssi;
    }
    doc["count"] = wiFiScanList.count;

    String output;
    serializeJson(doc, output);
    return output;
}