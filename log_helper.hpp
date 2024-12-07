#ifndef LOG_HELPER_HPP
#define LOG_HELPER_HPP

#include <Arduino.h>
#include <ArduinoLog.h>
#include <ESP8266WiFi.h>

// 声明日志级别名称
extern const char* logLevelNames[];

// WiFi状态本地化字符串
String localizableWLStatus(wl_status_t status);

// 打印前缀（时间戳、日志级别）
void printPrefix(Print* _logOutput, int logLevel);

// 检查并初始化日志库（如果尚未初始化）
bool initLog();

#endif // LOG_HELPER_HPP