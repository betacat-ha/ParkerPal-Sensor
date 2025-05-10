#ifndef VL53L0X_SENSOR_HPP
#define VL53L0X_SENSOR_HPP

#include <Adafruit_VL53L0X.h>
#include <vector>
#include <string>
#include "log_helper.hpp"

class VL53L0XSensor {
public:
    using OccupyStatusChangeCallback = std::function<void(VL53L0XSensor*)>;
    using SensorFailureCallback = std::function<void(VL53L0XSensor*)>;

    // 构造函数，支持创建多个传感器实例
    VL53L0XSensor(const std::string& id, int slot);
    ~VL53L0XSensor();


    // 静态方法，循环所有传感器实例
    static const void loopAllSensors(int interval = 3000);

    void loop();


    // 获取距离
    int getDistance();
    // 校准传感器
    void calibrate();

    // 设置占用状态变化回调
    void setOccupyStatusChangeCallback(OccupyStatusChangeCallback callback);

    // 设置传感器错误回调
    void setSensorFailureCallback(SensorFailureCallback callback);

    // 设置占用阈值
    void setOccupyThresholdMin(int threshold);
    void setOccupyThresholdMax(int threshold);

    // 获取当前占用状态
    int getOccupyStatus() const;

    // 获取上次上报的占用状态
    int getReportedOccupyStatus() const;

    // 获取校准距离
    int getDistanceCalibration() const;

    // 设置校准距离
    void setDistanceCalibration(int distance);

    // 获取传感器测距距离
    int getDistance() const;

    // 获取车位ID
    const std::string& getId() const;

    // 获取车位编号
    int getSlot() const;

    // 静态方法，获取所有传感器实例
    static const std::vector<VL53L0XSensor*>& getAllSensors();

private:
    Adafruit_VL53L0X _lox;

    OccupyStatusChangeCallback _occupyStatusChangeCallback;
    SensorFailureCallback _sensorFailureCallback;

    int modifyAddress(int slot);

    // 车位配置
    std::string _id;
    int _slot;

    // 车位占用判断阈值
    int _occupyThresholdMin = 140; // 判断车位占用的最低界限
    int _occupyThresholdMax = 280; // 判断车位占用的最高界限

    // 车位占用状态
    int _reportedOccupyStatus = -1; // 上一次上报给服务器的占用状态

    // 车位预约状态
    int _reservationStatus;

    // 传感器校准距离
    int _distanceCalibration;

    // 传感器测距距离
    int _distance = 0;

    struct VL53L0XDevice {
        uint8_t i2cAddress;
        uint8_t xshutPin;
    };

    static const VL53L0XDevice _devices[4];


    // 静态变量，用于存储所有传感器实例
    static std::vector<VL53L0XSensor*> _allSensors;
};

#endif // VL53L0X_SENSOR_HPP