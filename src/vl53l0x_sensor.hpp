#ifndef VL53L0X_SENSOR_HPP
#define VL53L0X_SENSOR_HPP

#include <Adafruit_VL53L0X.h>
#include "log_helper.hpp"

class VL53L0XSensor {
public:
    VL53L0XSensor();
    int getDistance();
    void calibrate();

private:
    Adafruit_VL53L0X _lox;
    int _distanceCalibration = 30;
};

#endif // VL53L0X_SENSOR_HPP