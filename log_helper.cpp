#include "log_helper.hpp"

const char* logLevelNames[] = {
    "【静默】", // LOG_LEVEL_SILENT
    "【致命】", // LOG_LEVEL_FATAL
    "【错误】", // LOG_LEVEL_ERROR
    "【警告】", // LOG_LEVEL_WARNING
    "【信息】", // LOG_LEVEL_INFO
    "【跟踪】", // LOG_LEVEL_TRACE
    "【啰嗦】"  // LOG_LEVEL_VERBOSE
};

// WiFi状态本地化字符串
String localizableWLStatus(wl_status_t status) {
  switch (status) {
    case WL_NO_SSID_AVAIL:  return "找不到该SSID";
    case WL_CONNECTED:      return "已连接";
    case WL_CONNECT_FAILED: return "连接失败";
    case WL_CONNECTION_LOST:return "失去连接";
    // case WL_WRONG_PASSWORD: return "错误的密码";
    case WL_DISCONNECTED:   return "已断开连接";
    case WL_IDLE_STATUS:    return "已休眠";
    default:                return "未知";
  }
}

/**
 * 检查并初始化日志库（如果尚未初始化）
 * @param level 日志级别
 * @return 是否初始化成功
 */
bool initLog(int level) {
    static bool initialized = false;
    if (!initialized) {
        // 设置日志前缀和后缀函数
        Log.setPrefix(printPrefix); // 使用自定义的前缀打印函数
        // Log.setSuffix([](Print* _logOutput, int logLevel) { _logOutput->println(); });

        // 初始化日志库
        Log.begin(level, &Serial);
        Log.setShowLevel(false); // 不显示内置日志级别，因为我们已经在前缀中处理了
        
        Log.noticeln("[Log] 日志框架初始化完成！");

        initialized = true;
        return true;
    }
    
    Log.errorln("[Log] 日志框架初始化失败！");
    return false;
}

void printLogLevel(Print* _logOutput, int logLevel) {
    if (logLevel >= 0 && logLevel <= 6) {
        _logOutput->print(logLevelNames[logLevel]);
    } else {
        _logOutput->print("【未知】");
    }

    _logOutput->print(": ");
}

// void printTimestamp(){

// }

// 打印前缀（时间戳、日志级别）
void printPrefix(Print* _logOutput, int logLevel) {
    // printTimestamp(_logOutput); // 如果需要实现时间戳功能
    printLogLevel(_logOutput, logLevel);
}