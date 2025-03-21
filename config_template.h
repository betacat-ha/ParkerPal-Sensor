// 本文件为模版，请新建config.h，并复制以下内容到其中修改。
#ifndef CONFIG_H
#define CONFIG_H

#define CONF_WIFI_SSID "your_wifi_ssid"
#define CONF_WIFI_PASSWORD "your_wifi_password"
#define CONF_WIFI_OVERRIDE_SMARTCONF false      // 是否覆盖智能配网的配置，使用上述SSID和密码连接AP
#define CONF_MQTT_SERVER_ADDRESS "your_mqtt_server_address"
#define CONF_MQTT_SERVER_USER "your_mqtt_server_user"
#define CONF_MQTT_SERVER_PASSWORD "your_mqtt_server_password"
#define CONF_MQTT_SERVER_PORT your_mqtt_server_port

// 调试设置
#define CONF_TEST_EXIT false                    // 硬件测试完成后是否退出
#define CONF_TEST_IGNORE_VL53L0X_FAILED false   // 是否忽略VL53L0X传感器初始化失败，并禁用一些功能

// 日志级别
#define CONF_LOG_LEVEL LOG_LEVEL_INFO           // 日志级别
#define LOG_LEVEL_SILENT  0
#define LOG_LEVEL_FATAL   1
#define LOG_LEVEL_ERROR   2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_INFO    4
#define LOG_LEVEL_NOTICE  4
#define LOG_LEVEL_TRACE   5
#define LOG_LEVEL_VERBOSE 6
#endif