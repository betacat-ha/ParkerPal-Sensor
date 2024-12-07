#ifndef JSON_HELPER_HPP
#define JSON_HELPER_HPP

#include <ArduinoJson.h>

struct ParkingSpaceStatus {
    String spaceName;
    int occupyStatus;
    int reservationStatus;
    float distance;
};

struct RSSIs {

};

String fromJsonStruct(const ParkingSpaceStatus& parkingSpaceStatus);

String fromJsonStruct(const RSSIs& rssis);

#endif // JSON_HELPER_HPP