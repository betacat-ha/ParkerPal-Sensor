/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Original Copyright(C) 2024 by ParkerPal
 */

// 本文件为模版，请新建config.h，并复制以下内容到其中修改。
#ifndef CONFIG_H
#define CONFIG_H

#define CONF_WIFI_SSID ""
#define CONF_WIFI_PASSWORD ""
#define CONF_WIFI_OVERRIDE_SMARTCONF false      // 是否覆盖智能配网的配置，使用上述SSID和密码连接AP
#define CONF_MQTT_SERVER_ADDRESS "your_mqtt_server_address"
#define CONF_MQTT_SERVER_USER "your_mqtt_server_user"
#define CONF_MQTT_SERVER_PASSWORD "your_mqtt_server_password"
#define CONF_MQTT_SERVER_PORT 1883

#define CONF_SWITCH_REST 13                     // 复位按键引脚

#define CONF_VL53L0X_ADDRESS ["0x29", "0x2A", "0x2B", "0x2C"] // VL53L0X传感器地址
#define CONF_VL53L0X_MODE 0                     // VL53L0X传感器工作模式，0表示单次测距，1表示连续测距

#define CONF_TX_PIN 16                       // ESP32-S3 TX引脚
#define CONF_RX_PIN 17                       // ESP32-S3 RX引脚  

// 调试设置
#define CONF_TEST_EXIT false                    // 硬件测试完成后是否退出
#define CONF_TEST_IGNORE_VL53L0X_FAILED false   // 是否忽略VL53L0X传感器初始化失败，并禁用一些功能

// 日志级别
#define CONF_LOG_LEVEL LOG_LEVEL_VERBOSE           // 日志级别
#define LOG_LEVEL_SILENT  0
#define LOG_LEVEL_FATAL   1
#define LOG_LEVEL_ERROR   2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_INFO    4
#define LOG_LEVEL_NOTICE  4
#define LOG_LEVEL_TRACE   5
#define LOG_LEVEL_VERBOSE 6

#define BL_Color 0x000000   //关---清空颜色
#define W_Color  0xFFFFFF   //白光
#define R_Color  0xFF0000   //红色
#define G_Color  0x00FF00   //绿色
#define B_Color  0x0000FF   //蓝色

#define LED_PIN    48     // WS2812B连接的GPIO引脚
#define LED_COUNT  1     // 灯珠数量（1颗）

#define COLOR_SENSOR 0
#define COLOR_WIFI 1
#define COLOR_MQTT 2
#define COLOR_CONFIG 3
#define COLOR_COMPLETE 4
#define COLOR_ERROR 5
#define COLOR_SERIAL 6
#define COLOR_BLACK 7
#endif