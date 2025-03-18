#include "vl53l0x_sensor.hpp"

VL53L0XSensor::VL53L0XSensor() {
    if (!_lox.begin()) {
        Log.errorln("[VL53L0X] 无法启动！");
    }
}

int VL53L0XSensor::getDistance() {
    VL53L0X_RangingMeasurementData_t measure;
    if (_lox.begin()) {
        _lox.rangingTest(&measure, false);
        return measure.RangeMilliMeter;
      }
    
      // 未知异常返回-1
    return -1;
}

void VL53L0XSensor::calibrate() {
    Log.noticeln("[VL53L0X] 开始校准...");

    int result = 0;
    int times = 0;
    int successfulTimes = 0;

    while (times <= 10 && successfulTimes <= 3) {
        VL53L0X_RangingMeasurementData_t measure;
        _lox.rangingTest(&measure, false);
        if (measure.RangeStatus != 4) {
            if (result == 0) {
                result = measure.RangeMilliMeter;
            } else {
                result = (measure.RangeMilliMeter + result) / 2;
            }
            successfulTimes++;
        }
        times++;
    }

    if (result != 0) {
        _distanceCalibration = result;
        Log.noticeln("[VL53L0X] 校准完成！");
    } else {
        Log.errorln("[VL53L0X] 校准失败！");
    }
}