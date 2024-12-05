#ifndef OPERATIONS_H
#define OPERATIONS_H

// 定义 OperationCode 枚举
enum OperationCode {
    OPERATION_UNKNOWN = 0,
    OPERATION_MODIFY_SETTINGS,    // 修改设置
    OPERATION_SYNIC_TIME,         // 同步时间
    OPERATION_REBOOT,             // 重启设备
    OPERATION_RESET,              // 重置传感器
    OPERATION_NOTIFY_ALERT,       // 发送警报通知
    OPERATION_NOTIFY_LED,         // 闪烁LED指示灯
    OPERATION_CHECK_STATUS,       // 上传状态信息
    OPERATION_UPDATE_FIRMWARE,    // 更新固件
    OPERATION_TOGGLE_SENSOR_MODE, // 切换传感器工作模式
    OPERATION_CALIBRATE_SENSOR    // 校准传感器
};

// 声明 getOperationCode 函数
OperationCode getOperationCode(const char* operation);

#endif // OPERATIONS_H