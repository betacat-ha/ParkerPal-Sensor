#include "ArduinoLog.h"
#include "AtHandler.h"

AtHandler::AtHandler(HardwareSerial& serial) : espSerial(serial) {}

void AtHandler::begin(unsigned long baud) {
    espSerial.begin(baud);
}

void AtHandler::onOK(Callback cb) {
    okCallback = cb;
}

void AtHandler::onMQTTMessage(MQTTCallback cb) {
    mqttCallback = cb;
}

void AtHandler::handleMQTTMessage(const String& line) {
    if (mqttCallback) {
        int topicStart = line.indexOf('"');
        int topicEnd = line.indexOf('"', topicStart + 1);
        String topic = line.substring(topicStart + 1, topicEnd);

        int payloadStart = line.indexOf('{');
        int payloadEnd = line.lastIndexOf('}');
        String payload = line.substring(payloadStart, payloadEnd + 1);

        Log.verboseln("[AT] 处理MQTT消息：主题：%s，有效载荷：%s", topic.c_str(), payload.c_str());

        // 调用用户定义的回调函数
        mqttCallback(topic.c_str(), payload.c_str());
    }
}

void AtHandler::onCustom(const String& keyword, Callback cb) {
    customCallbacks[keyword] = cb;
}

void AtHandler::loop() {
    while (espSerial.available()) {
        char c = espSerial.read();
        buffer += c;
        if (buffer.endsWith("\r\n")) {
            processLine(buffer);
            buffer = "";
        }
    }
}

void AtHandler::processLine(const String& line) {
    Log.verboseln("[AT] 接收到消息：% s", line.c_str());
    if (line.startsWith("OK")) {
        if (okCallback) okCallback();
    } else if (line.startsWith("+MQTTSUBRECV:")) {
        handleMQTTMessage(line);
    } else {
        for (const auto& entry : customCallbacks) {
            if (line.startsWith(entry.first)) {
                entry.second();
                break;
            }
        }
    }
}

int AtHandler::sendATCommand(const String& command, const String& expectedResponse, unsigned long timeout) {
    espSerial.println(command.c_str());
    Log.verboseln("[AT] 发送消息：%s", command.c_str());

    String response;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while (espSerial.available()) {
            char c = espSerial.read();
            response += c;
            if (response.indexOf(expectedResponse) != -1) {
                Log.verboseln("[AT] 接收到响应：%s", response.c_str());
                return 0;  // success
            } else if (response.indexOf("ERROR") != -1) {
                Log.errorln("[AT] 接收到错误响应：%s", response.c_str());
                return -1; // error
            }
        }
    }
    Log.errorln("[AT] 超时：%s", command.c_str());
    return -2; // timeout
}

int AtHandler::sendATCommandWithPayload(const String& command, const uint8_t* payload, size_t length, const String& successResponse, const String& failureResponse, unsigned long timeout) {
    espSerial.println(command);
    Log.verboseln("[AT] 发送命令：%s", command.c_str());

    // 等待 '>' 提示符
    unsigned long start = millis();
    while (millis() - start < timeout) {
        if (espSerial.available()) {
            char c = espSerial.read();
            if (c == '>') {
                break; // 准备发送数据
            }
        }
    }

    // 超时未收到 '>' 提示符
    if (millis() - start >= timeout) {
        Log.errorln("[AT] 等待 '>' 超时");
        return -2;
    }

    // 发送有效载荷
    espSerial.write(payload, length);
    Log.verboseln("[AT] 已发送有效载荷，长度：%d 字节", length);

    // 等待响应，但不中断中间处理
    String line;
    start = millis();
    while (millis() - start < timeout) {
        while (espSerial.available()) {
            line = espSerial.readStringUntil('\n');
            if (line.length() > 0) {
                line.trim();

                if (line.startsWith(successResponse)) {
                    Log.infoln("[AT] 成功响应：%s", line.c_str());
                    delay(100);
                    return 0;
                } else if (line.startsWith(failureResponse)) {
                    Log.errorln("[AT] 失败响应：%s", line.c_str());
                    delay(100);
                    return -1;
                } else if (line.startsWith("+MQTTSUBRECV:")) {
                    handleMQTTMessage(line);  // 实时处理订阅消息 
                } else {
                    Log.verbose("[AT] 其他响应：%s", line.c_str());
                }
                line = "";
            }
        }
    }

    Log.errorln("[AT] 超时未收到成功/失败响应，命令：%s", command.c_str());
    return -2; // 超时
}

int AtHandler::waitForResponse(const String& expectedKeyword, String& rawResponse, unsigned long timeout) {
    rawResponse = "";
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while (espSerial.available()) {
            char c = espSerial.read();
            rawResponse += c;
            if (rawResponse.indexOf(expectedKeyword) != -1) {
                return 0;
            } else if (rawResponse.indexOf("ERROR") != -1) {
                return -1;
            }
        }
    }
    return -2;
}

int AtHandler::connectWiFi() {
    return connectWiFi("", "", true);
}

int AtHandler::connectWiFi(const String& ssid, const String& password) {
    return connectWiFi(ssid, password, false);
}

int AtHandler::connectWiFi(const String& ssid, const String& password, bool overrideSmartConfig) {
    // 设置WiFi模式为Station模式
    setWifiMode(1);
    String cmd;
    char buffer[64];
    if (overrideSmartConfig) {
        cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
        snprintf(buffer, sizeof(buffer), "[AT] 连接到 %s, 密码 %s", ssid.c_str(), password.c_str());
    } else {
        cmd = "AT+CWJAP";
        snprintf(buffer, sizeof(buffer), "[AT] 连接到上次配置的AP");
    }

    Log.noticeln("%s", buffer);
    espSerial.println(cmd);

    String response;
    unsigned long start = millis();
    while (millis() - start < 10000) {  // 增加超时时间
        while (espSerial.available()) {
            char c = espSerial.read();
            response += c;

            if (response.indexOf("WIFI CONNECTED") != -1 &&
                response.indexOf("WIFI GOT IP") != -1 &&
                response.indexOf("OK") != -1) {
                Log.noticeln("[AT] Wi-Fi连接成功");
                return 0;  // 成功
            } else if (response.indexOf("+CWJAP:") != -1) {
                Log.error("[AT] Wi-Fi连接失败");
                if (overrideSmartConfig) {
                    Log.errorln("[AT] 无法连接至Wi-Fi.");
                    return -1;  // 连接失败
                } else {
                    Log.errorln("[AT] 启动智能配网...");
                    return smartConfig();  // 启动智能配网
                }
                return response.indexOf("ERROR") != -1 ? -1 : -2;  // 连接失败
            }
        }
    }
    return -3;  // 超时
}

int AtHandler::startSmartConfig() {
    sendATCommand("AT+CWMODE=1");
    return sendATCommand("AT+CWSTARTSMART=3");
}

int AtHandler::stopSmartConfig() {
    return sendATCommand("AT+CWSTOPSMART");
}

int AtHandler::smartConfig(int retryTimes) {
    Log.noticeln("[AT] 启动智能配网...");

    while(retryTimes--) {
        startSmartConfig();
        String response;
        unsigned long start = millis();
        while (millis() - start < 30000) {
            while (espSerial.available()) {
                char c = espSerial.read();
                response += c;
                if (response.indexOf("WIFI CONNECTED") != -1 &&
                    response.indexOf("WIFI GOT IP") != -1) {
                    Log.noticeln("[AT] 智能配网成功");

                    delay(3000);
                    stopSmartConfig();
                    return 0;  // 成功
                }
            }
        }
        delay(1000);
    }
    Log.errorln("[AT] 智能配网失败");
    stopSmartConfig();
    return -1;  // 失败
}

int AtHandler::mqttConnect(const String& broker, int port, const String& clientId, const String& username, const String& password) {
    String cfg = "AT+MQTTUSERCFG=0,1,\"" + String(clientId) + "\",\"" + username + "\",\"" + password + "\",0,0,\"\"";
    Log.verboseln("[AT] clientId：%s, username：%s", String(clientId), username);
    Log.verboseln("[AT] 设置MQTT服务器：%s", cfg.c_str());
    int r1 = sendATCommand(cfg.c_str());
    if (r1 != 0) return r1;
    String conn = "AT+MQTTCONN=0,\"" + broker + "\"," + String(port) +  ",1";
    Log.verboseln("[AT] 连接MQTT服务器： %s ", conn.c_str());
    return sendATCommand(conn.c_str());
}

int AtHandler::mqttSubscribe(const String& topic) {
    String cmd = "AT+MQTTSUB=0,\"" + topic + "\",1";
    Log.verboseln("[AT] 订阅MQTT主题：%s", cmd.c_str());
    return sendATCommand(cmd);
}

int AtHandler::mqttPublish(const String& topic, const String& message) {
    // 转义引号
    String escapedMessage = message;
    // String escapedMessage = "{\"type\":\"config\"}";
    escapedMessage.replace("\"", "\\\"");
    escapedMessage.trim();

    String cmd = "AT+MQTTPUB=0,\"" + topic + "\",\"" + escapedMessage + "\",1,0";

    Log.verbose("[AT] 发送MQTT消息：%s 十六进制 ", cmd.c_str());
    for (int i = 0; i < cmd.length(); i++) {
        Serial.print(cmd[i], DEC);
        Serial.print(" ");
    }
    Serial.println();

    return sendATCommand(cmd);
}

int AtHandler::mqttPublishWithRaw(const String& topic, const uint8_t* data, size_t length, unsigned long timeout) {
    // 构建 AT 命令
    String command = "AT+MQTTPUBRAW=0,\"" + topic + "\"," + length + ",1,0";

    // 调用通用方法
    return sendATCommandWithPayload(command, data, length, "+MQTTPUB:OK", "+MQTTPUB:FAIL", timeout);
}


int AtHandler::getWiFiStatus(String& status, String& ssid) {
    espSerial.println("AT+CWSTATE?");
    String response;
    int result = waitForResponse("+CWSTATE:", response);
    if (result != 0) return result;

    int startIdx = response.indexOf(":") + 1;
    int commaIdx = response.indexOf(',', startIdx);
    int quoteStart = response.indexOf('"', commaIdx);
    int quoteEnd = response.indexOf('"', quoteStart + 1);

    if (startIdx != -1 && commaIdx != -1 && quoteStart != -1 && quoteEnd != -1) {
        status = response.substring(startIdx, commaIdx);
        ssid = response.substring(quoteStart + 1, quoteEnd);
        return 0;
    }
    return -1;
}

int AtHandler::setWifiMode(int mode) {
    String cmd = "AT+CWMODE=" + String(mode);
    return sendATCommand(cmd);
}

int AtHandler::getMacAddress(String& mac) {
    espSerial.println("AT+CIPSTAMAC?");
    String response;
    int result = waitForResponse("+CIPSTAMAC:", response);
    if (result != 0) return result;

    int startIdx = response.indexOf(":") + 1;
    int endIdx = response.indexOf('\r', startIdx);
    if (startIdx != -1 && endIdx != -1) {
        mac = response.substring(startIdx, endIdx);
        return 0;
    }
    return -1;
}

int AtHandler::enableEcho() {
    return sendATCommand("ATE1");
}

int AtHandler::disableEcho() {
    return sendATCommand("ATE0");
}

int AtHandler::enableSysLog() {
    return sendATCommand("AT+SYSLOG=1");
}

int AtHandler::disableSysLog() {
    return sendATCommand("AT+SYSLOG=0");
}