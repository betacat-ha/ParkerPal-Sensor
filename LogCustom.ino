String localizableWLStatus(wl_status_t status) {
  switch (status) {
    case WL_NO_SSID_AVAIL:
      return "找不到该SSID";
    case WL_CONNECTED:
      return "已连接";
    case WL_CONNECT_FAILED:
      return "连接失败";
    case WL_CONNECTION_LOST:
      return "失去连接";
    case WL_WRONG_PASSWORD:
      return "错误的密码";
    case WL_DISCONNECTED:
      return "已断开连接";
    case WL_IDLE_STATUS:
      return "已休眠";
    default:
      return "未知";
  }
  return "未知";
}

void printPrefix(Print* _logOutput, int logLevel) {
    // printTimestamp(_logOutput);
    printLogLevel (_logOutput, logLevel);
}

void printLogLevel(Print* _logOutput, int logLevel) {
    /// Show log description based on log level
    switch (logLevel)
    {
        default:
        case 0:_logOutput->print("【静默】"); break;
        case 1:_logOutput->print("【致命】"); break;
        case 2:_logOutput->print("【错误】"); break;
        case 3:_logOutput->print("【警告】"); break;
        case 4:_logOutput->print("【信息】"); break;
        case 5:_logOutput->print("【跟踪】"); break;
        case 6:_logOutput->print("【啰嗦】"); break;
    }   
}