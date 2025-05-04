#ifndef CROSS_PLATFORM_HPP
#define CROSS_PLATFORM_HPP

// 跨平台兼容
#ifdef ESP32
#define WIFI_LIBRARY <WiFi.h>
#define INTERNAL_STORAGE_LIBRARY <nvs_flash.h>
#define WIFI_ENCRYPTION_OPEN WIFI_AUTH_OPEN
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#elif defined(ESP8266)
#define WIFI_LIBRARY <ESP8266WiFi.h>
#define INTERNAL_STORAGE_LIBRARY <EEPROM.h>
#define WIFI_ENCRYPTION_OPEN ENC_TYPE_NONE
#define I2C_SDA_PIN D1
#define I2C_SCL_PIN D2
#else
#error "Unsupported platform"
#endif



#endif