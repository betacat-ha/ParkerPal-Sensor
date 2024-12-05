#include "operations.h"
#include <string.h> // 用于 strcmp 函数

OperationCode getOperationCode(const char* operation) {
    if (strcmp(operation, "modify_settings") == 0) {
        return OPERATION_MODIFY_SETTINGS;
    } else if (strcmp(operation, "sync_time") == 0) {
        return OPERATION_SYNIC_TIME;
    } else if (strcmp(operation, "reboot") == 0) {
        return OPERATION_REBOOT;
    } else if (strcmp(operation, "notify_led") == 0) {
        return OPERATION_NOTIFY_LED;
    } else if (strcmp(operation, "check_status") == 0) {
        return OPERATION_CHECK_STATUS;
    } else if (strcmp(operation, "reset") == 0) {
        return OPERATION_RESET;
    } else if (strcmp(operation, "notify_alert") == 0) {
        return OPERATION_NOTIFY_ALERT;
    } else if (strcmp(operation, "update_firmware") == 0) {
        return OPERATION_UPDATE_FIRMWARE;
    } else if (strcmp(operation, "toggle_sensor_mode") == 0) {
        return OPERATION_TOGGLE_SENSOR_MODE;
    } else if (strcmp(operation, "calibrate_sensor") == 0) {
        return OPERATION_CALIBRATE_SENSOR;
    } else {
        return OPERATION_UNKNOWN;  // 未知的操作
    }
}