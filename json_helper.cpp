#include "json_helper.hpp"

/**
 * @brief 转换parkingSpaceStatus结构体为JSON字符串
 * @param parkingSpaceStatus
 * @return JSON字符串
 */
String fromJsonStruct(const ParkingSpaceStatus& parkingSpaceStatus) {
    StaticJsonDocument<256> doc;
    doc["type"] = "VL53L0X";
    doc["spaceName"] = parkingSpaceStatus.spaceName.c_str();
    doc["occupyStatus"] = parkingSpaceStatus.occupyStatus;
    doc["reservationStatus"] = parkingSpaceStatus.reservationStatus;
    doc["distance"] = parkingSpaceStatus.distance;

    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * @brief 转换wiFiScanList结构体为JSON字符串
 * @param wiFiScanList
 * @return JSON字符串
 */
String fromJsonStruct(const WiFiScanList& wiFiScanList) {
    DynamicJsonDocument doc(1024);
    doc["type"] = "Wi-Fi";

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

/**
 * @brief 转换parkingSpaceStatus结构体为JsonObject
 * @param doc
 * @param parkingSpaceStatus
 * @return JsonObject
 */
JsonObject getJsonObject(JsonDocument& doc, const ParkingSpaceStatus& parkingSpaceStatus) {
    doc.clear();

    doc["type"] = "VL53L0X";
    doc["spaceName"] = parkingSpaceStatus.spaceName.c_str();
    doc["occupyStatus"] = parkingSpaceStatus.occupyStatus;
    doc["reservationStatus"] = parkingSpaceStatus.reservationStatus;
    doc["distance"] = parkingSpaceStatus.distance;
    return doc.to<JsonObject>();
}

/**
 * @brief 转换wiFiScanList结构体为JsonObject
 * @param doc
 * @param wiFiScanList
 * @return JsonObject
 */
JsonObject getJsonObject(JsonDocument& doc, const WiFiScanList& wiFiScanList) {
    doc.clear();

    doc["type"] = "Wi-Fi";

    JsonArray networksArray = doc.createNestedArray("networks");

    for (int i = 0; i < wiFiScanList.count; ++i) {
        JsonObject networkObj = networksArray.createNestedObject();
        networkObj["ssid"] = wiFiScanList.networks[i].ssid.c_str();
        networkObj["rssi"] = wiFiScanList.networks[i].rssi;
    }
    doc["count"] = wiFiScanList.count;

    return doc.to<JsonObject>();
}

/**
 * 将 JSON 字符串解析为 JsonObject
 * @param jsonString 输入的 JSON 字符串
 * @return 返回解析得到的 JsonObject，解析失败时返回包含占位信息的 JsonObject
 */
JsonObject stringToJsonObject(JsonDocument& doc, const String& jsonString) {
    // 清空文档，确保内容不会混淆
    doc.clear();

    // 尝试解析 JSON 字符串
    DeserializationError error = deserializeJson(doc, jsonString);

    // 如果解析失败，返回带有占位信息的对象
    if (error) {
        JsonObject placeholder = doc.to<JsonObject>();
        placeholder["error"] = "Invalid JSON";
        return placeholder;
    }

    // 返回解析成功的 JsonObject
    return doc.as<JsonObject>();
}