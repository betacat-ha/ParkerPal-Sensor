#include "vl53l0x_sensor.hpp"

// 静态成员初始化
std::vector<VL53L0XSensor*> VL53L0XSensor::_allSensors;

// 定义传感器设备信息
const VL53L0XSensor::VL53L0XDevice VL53L0XSensor::_devices[4] = {
    {0x29, 1}, // 传感器0
    {0x2A, 2}, // 传感器1
    {0x2B, 3}, // 传感器2
    {0x2C, 18} // 传感器3
};

VL53L0XSensor::VL53L0XSensor(const std::string& id, int slot)
    : _id(id), _slot(slot) {
        Log.infoln("[VL53L0X] 初始化传感器 %s", id.c_str());
        VL53L0XDevice device = _devices[slot];
        //  修改I2C地址
        if (modifyAddress(slot) != 0) {
            Log.errorln("[VL53L0X] %s 号传感器地址修改失败！", id.c_str());
            return;
        }
    _allSensors.push_back(this);
}

VL53L0XSensor::~VL53L0XSensor() {
    auto it = std::find(_allSensors.begin(), _allSensors.end(), this);
    if (it != _allSensors.end()) {
        _allSensors.erase(it);
    }
}

const void VL53L0XSensor::loopAllSensors() {
    for (auto& sensor : VL53L0XSensor::getAllSensors()) {
        sensor->loop();
    }
}

int VL53L0XSensor::modifyAddress(int slot) {
    int deviceCount = sizeof(_devices) / sizeof(_devices[0]);
    // 配置所有 XSHUT 引脚为输出并拉低（关闭所有 VL53L0X）
    for (int i = 0; i < deviceCount; i++) {
        pinMode(_devices[i].xshutPin, OUTPUT);
        digitalWrite(_devices[i].xshutPin, LOW);
    }
    // 等待 100ms
    delay(100);

    // 打开当前传感器的 XSHUT 引脚
    pinMode(_devices[slot].xshutPin, OUTPUT);
    digitalWrite(_devices[slot].xshutPin, HIGH);

    // 等待 100ms
    delay(100);

    if (!_lox.begin()) {
        Log.errorln("[VL53L0X] %d 号传感器无法启动！", slot);
        return -1;
    }

    _lox.setAddress(_devices[slot].i2cAddress);
    
    return 0;
}

void VL53L0XSensor::loop() {
    if (!_lox.begin()) {
        Log.errorln("[VL53L0X] %d 号传感器无法启动！", _slot);
        if (_sensorFailureCallback) {
            _sensorFailureCallback(this);
        }
        return;
    }

    _distance = getDistance();

    if (_distance == -1) {
        Log.errorln("[VL53L0X] %d 号传感器测距失败！", _slot);
        if (_sensorFailureCallback) {
            _sensorFailureCallback(this);
        }
        return;
    }

    // 判断车位占用状态
    if (getOccupyStatus() != _reportedOccupyStatus) {
        _reportedOccupyStatus = getOccupyStatus();
        // 调用回调函数
        if (_occupyStatusChangeCallback) {
            _occupyStatusChangeCallback(this);
        }
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

void VL53L0XSensor::setOccupyStatusChangeCallback(OccupyStatusChangeCallback callback) {
    _occupyStatusChangeCallback = callback;
}

void VL53L0XSensor::setSensorFailureCallback(SensorFailureCallback callback) {
    _sensorFailureCallback = callback;
}

void VL53L0XSensor::setOccupyThresholdMin(int threshold) {
    _occupyThresholdMin = threshold;
}

void VL53L0XSensor::setOccupyThresholdMax(int threshold) {
    _occupyThresholdMax = threshold;
}

int VL53L0XSensor::getOccupyStatus() const {
    if (_distance >= _occupyThresholdMin && _distance <= _occupyThresholdMax) {
        return 1; // 占用
    } else {
        return 0; // 空闲
    }
}

int VL53L0XSensor::getReportedOccupyStatus() const {
    return _reportedOccupyStatus;
}

int VL53L0XSensor::getDistanceCalibration() const {
    return _distanceCalibration;
}

void VL53L0XSensor::setDistanceCalibration(int distance) {
    _distanceCalibration = distance;
}

const std::string& VL53L0XSensor::getId() const {
    return _id;
}

int VL53L0XSensor::getSlot() const {
    return _slot;
}

const std::vector<VL53L0XSensor*>& VL53L0XSensor::getAllSensors() {
    return _allSensors;
}