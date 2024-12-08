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

String fromJsonStruct(const RSSIs& rssis) {
    return "test";
}