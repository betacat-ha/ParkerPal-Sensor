/*项目名称：智泊无忧传感器端
  开发团队：智泊无忧硬件组
  立项时间：2024.8.30
*/
#include <Arduino.h>
#include "cross_platform.hpp"
#include "json_helper.hpp"
#include "log_helper.hpp"
#include "mqtt_handler.hpp"
#include "AtHandler.h"
#include "operations.hpp"
#include "system.hpp"
#include "vl53l0x_sensor.hpp"
#include "wifi_handler.hpp"
#include <ArduinoLog.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

void callbackMqtt(const char* topic, const char* message);
void callbackMqttByPayload(char* topic, byte* payload, unsigned int length);
// 配置文件
#if __has_include("config.h")
// 如果存在 config.h，则优先包含
#include "config.h"
#else
// 如果不存在 config.h，则回退到 config_template.h
#include "config_template.h"
#endif

//=======================基础设置==========================
// 串口通信部分
int SerialData = 0; //串口传入数据
String comdata = "";
SystemSettings settings;
HardwareSerial ESPSerial(2); // 使用硬件串口2
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//=======================物联网部分============================
// const char * WIFI_CHANNEL                       // Wi-Fi接入点的频道，可选
// const char * WIFI_BSSID                         // Wi-Fi接入点的MAC地址，可选
WiFiHandler *wifiHandler = nullptr;
AtHandler *atHandler = nullptr;

const char *MQTT_SERVER_ADDRESS = CONF_MQTT_SERVER_ADDRESS;   // MQTT服务器地址
const char *MQTT_SERVER_USER = CONF_MQTT_SERVER_USER;         // MQTT服务器用户
const char *MQTT_SERVER_PASSWORD = CONF_MQTT_SERVER_PASSWORD; // MQTT服务器密码
constexpr uint16_t MQTT_SERVER_PORT = CONF_MQTT_SERVER_PORT;
MQTTHandler *mqttHandler = nullptr;

//=======================车位检测部分============================
VL53L0XSensor *sensor = nullptr;

// 车位占用判断阈值
constexpr int OCCUPY_THRESHOLD_MIN = 140; // 判断车位占用的最低界限
constexpr int OCCUPY_THRESHOLD_MAX = 280; // 判断车位占用的最高界限

// String spaceId = "70162";
// String spaceName = "A-035"; // 车位名字
int occupyStatus = 0;         // 占用状态，0表示未占用；1表示被占用；
int reportedOccupyStatus = 0; // 上一次上报给服务器的占用状态
int reservationStatus = 0;    // 预约状态，0表示未被预约；1表示已被预约

uint32_t initColors[] = {
  strip.Color(255,0,255),     // 洋红色：初始化传感器
  strip.Color(0, 255, 0),     // 绿色：初始化网络
  strip.Color(0, 0, 255),     // 蓝色：连接服务器
  strip.Color(255, 255, 0),   // 黄色：加载配置
  strip.Color(255, 255, 255), // 白色：完成初始化
  strip.Color(255, 0, 0),     // 红色：错误
  strip.Color(85,107,47),     // 深橄榄绿色：初始化串口
  strip.Color(0,0,0),         // 关闭
};

void setup() {
    strip.begin();
    strip.setBrightness(50);
    strip.setPixelColor(0, initColors[COLOR_SERIAL]);
    strip.show();
    //=====================初始化串口==============================
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.begin(115200);

    // 检查复位按键是否被按下
    pinMode(CONF_SWITCH_REST, INPUT_PULLUP);
    if (digitalRead(CONF_SWITCH_REST) == LOW) {
        delay(3000);
        if (digitalRead(CONF_SWITCH_REST) == LOW) {
            Serial.print("\n[System] 复位按键被按下，清除设备配置...");
            eraseAllConfig();
            delay(1000);
            ESP.restart();
        }
    }

    delay(1000);

    // 出现此消息代表串口通信正常
    Serial.print("\n\n");
    Serial.print("================================\n");
    Serial.print("          智泊无忧传感器          \n");
    Serial.print("================================\n");

    //=====================初始化日志框架==========================
    initLog(CONF_LOG_LEVEL);

    //=====================初始化通信=============================
    // TODO 缺少重试
    // wifiHandler = new WiFiHandler(CONF_WIFI_SSID, CONF_WIFI_PASSWORD);

    // if (!wifiHandler->connect(CONF_WIFI_OVERRIDE_SMARTCONF)) {
    //     Log.errorln("[Wi-Fi] 无法建立与AP的连接。");
    //     // 如果Wi-Fi连接失败，进入死循环
    //     while (1) {
    //         // system_soft_wdt_feed(); // 喂狗，防止复位
    //     }
    // }

    // 初始化联网模块串口
    strip.setPixelColor(0, initColors[COLOR_WIFI]);
    strip.show();
    ESPSerial.begin(115200, SERIAL_8N1, CONF_RX_PIN, CONF_TX_PIN);
    ESPSerial.println("AT");
    if (ESPSerial.find("OK")) {
        Log.noticeln("[AT] ESP AT模块初始化成功！");
    } else {
        Log.errorln("[AT] ESP AT模块初始化失败！");
        strip.setPixelColor(0, initColors[COLOR_ERROR]);
        strip.show();
        while (1) {
            // system_soft_wdt_feed(); // 喂狗，防止复位
        }
    }

    atHandler = new AtHandler(ESPSerial);
    // atHandler->disableEcho(); // 禁用回显
    atHandler->enableSysLog();

    // 初始化Wi-Fi连接
    if (!atHandler->connectWiFi(CONF_WIFI_SSID, CONF_WIFI_PASSWORD, CONF_WIFI_OVERRIDE_SMARTCONF)) {
        Log.errorln("[AT] 无法建立与AP的连接。");
        // 如果Wi-Fi连接失败，进入死循环
        strip.setPixelColor(0, initColors[COLOR_ERROR]);
        strip.show();
        while (1) {
            // system_soft_wdt_feed(); // 喂狗，防止复位
        }
    }

    strip.setPixelColor(0, initColors[COLOR_CONFIG]);
    strip.show();

    bool deviceConfigured = isDeviceConfigured();

    if (!deviceConfigured) {
        // 请求配置
        Log.noticeln("[System] 设备未配置，MQTT服务器使用默认配置。");

        settings.deviceSettings.deviceMAC = "68:C6:3A:FB:E6:17";

        settings.mqttSettings.serverIP = CONF_MQTT_SERVER_ADDRESS;
        settings.mqttSettings.serverPort = CONF_MQTT_SERVER_PORT;
        settings.mqttSettings.serverUser = CONF_MQTT_SERVER_USER;
        settings.mqttSettings.serverPassword = CONF_MQTT_SERVER_PASSWORD;
    } else {
        // 加载配置
        loadConfig(settings);
    }

    // 如果Wi-Fi连接成功，则初始化MQTTHandler
    strip.setPixelColor(0, initColors[COLOR_MQTT]);
    strip.show();


    String clientId = "Sensor-" + settings.deviceSettings.deviceMAC;
    Log.verboseln("[Init] settings.deviceSettings.deviceMAC: %s",
              settings.deviceSettings.deviceMAC.c_str());
    String subTopicString = "/parkerpal/Sensor-Sub-" + settings.deviceSettings.deviceMAC;
    String pubTopicString = "/parkerpal/Sensor-Pub-" + settings.deviceSettings.deviceMAC;
    // mqttHandler = new MQTTHandler(settings.mqttSettings.serverIP.c_str(), 
    //                               settings.mqttSettings.serverPort,
    //                               settings.mqttSettings.serverUser.c_str(), 
    //                               settings.mqttSettings.serverPassword.c_str(),
    //                               clientId.c_str(),
    //                               subTopicString.c_str(),
    //                               pubTopicString.c_str());
    // mqttHandler->setCallback(callbackMqtt);
    // mqttHandler->setBufferSize(5120);
    // 
    // if (!mqttHandler->connect()) {
    //     Log.errorln("[MQTT] 无法建立与服务器的连接。");
    //     // 如果MQTT连接失败，进入死循环
    //     while (1) {
    //         // system_soft_wdt_feed(); // 喂狗，防止复位
    //     }
    // }

    // mqttHandler->subscribeTopic();

    // // 请求服务器配置
    // mqttHandler->publishMessage(("{\"type\":\"configuration_request\", \"deviceMacAddress\":\"" + settings.deviceSettings.deviceMAC + "\"}").c_str());

    atHandler->onMQTTMessage(callbackMqtt);

    if (atHandler->mqttConnect(settings.mqttSettings.serverIP.c_str(), 
                                settings.mqttSettings.serverPort, 
                                clientId, 
                                settings.mqttSettings.serverUser.c_str(), 
                                settings.mqttSettings.serverPassword.c_str()) != 0) {
        Log.errorln("[AT] 无法建立与MQTT服务器的连接。");
        strip.setPixelColor(0, initColors[COLOR_ERROR]);
        strip.show();
        // 如果MQTT连接失败，进入死循环
        while (1) {
            // system_soft_wdt_feed(); // 喂狗，防止复位
        }
    }

    // 订阅主题
    while (atHandler->mqttSubscribe(subTopicString.c_str()) != 0) {
        Log.errorln("[AT] 无法订阅主题。");
        strip.setPixelColor(0, initColors[COLOR_ERROR]);
        strip.show();
    };

    // 请求服务器配置
    String requestMessage = "{\"type\":\"configuration_request\",\"deviceMacAddress\":\"" + settings.deviceSettings.deviceMAC + "\"}";
    if (atHandler->mqttPublishWithRaw(pubTopicString.c_str(), (const uint8_t *)requestMessage.c_str(), strlen(requestMessage.c_str())) != 0) {
        Log.errorln("[AT] 无法发布配置请求。");
        strip.setPixelColor(0, initColors[COLOR_ERROR]);
        strip.show();
        while (1) {
            /* code */
        }
        
    }
    
    setDeviceUnConfigured();

    Log.noticeln("[System] 等待服务器配置...");
    while (!isDeviceConfigured()) {
        // mqttHandler->loop();    // 保持MQTT心跳
        atHandler->loop();    // 检查AT消息
        // system_soft_wdt_feed(); // 喂狗，防止复位
        delay(1000);
    }
    Log.noticeln("[System] 服务器配置成功！");
    loadConfig(settings);

    // 如果是首次配置，重启 
    if (!deviceConfigured) {
        Log.noticeln("[System] 首次配置完成，重启设备。");
        delay(1000);
        ESP.restart();
    }
    
    Log.noticeln("[System] 设备已配置，设备名：%s",
        settings.deviceSettings.deviceName.c_str());
    Log.noticeln("[System] 停车位信息：共%d个",
        settings.parkingSpaceList.count);
    for (int i = 0; i < settings.parkingSpaceList.count; ++i) {
        Log.verboseln(
            "[System] 车位%d：id(%s),name(%s)", i + 1,
            settings.parkingSpaceList.spaces[i].id,
            settings.parkingSpaceList.spaces[i].spaceName.c_str());
    }

    //=====================初始化激光传感器========================
    strip.setPixelColor(0, initColors[COLOR_SENSOR]);
    strip.show();
    sensor = new VL53L0XSensor();
    if (!sensor) {
        Log.errorln("[VL53L0X] 初始化失败！");
        strip.setPixelColor(0, initColors[COLOR_ERROR]);
        strip.show();
        while (1 && !CONF_TEST_IGNORE_VL53L0X_FAILED) {
            // system_soft_wdt_feed(); // 喂狗，防止复位
        }
    }

    //同步系统时间
    syncSystemTime();

    //========================初始化完成==========================
    strip.setPixelColor(0, initColors[COLOR_COMPLETE]);
    strip.show();

    delay(1000);
    strip.setPixelColor(0, initColors[COLOR_BLACK]);
    strip.show();
}

void loop() {
    /*
      ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
      ┃                                                            ┃
      ┃                                                            ┃
      ┃                          Key_read                          ┃
      ┃                                                            ┃
      ┃                                                            ┃
      ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
    */

    /*
      ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
      ┃                                                            ┃
      ┃                                                            ┃
      ┃                       Serial_read                          ┃
      ┃                                                            ┃
      ┃                                                            ┃
      ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
    */

    static unsigned long lastPublishTime = 0;
    constexpr unsigned long publishInterval = 3000; // 每3秒发布一次数据

    // WiFiScanList list = wifiHandler->scanNetworks();
    // String listJson = fromJsonStruct(list);
    // Log.verboseln("[Wi-Fi] AP列表：%s", listJson.c_str());
    // mqttHandler->publishMessage(listJson.c_str());

    // TODO: 这里要适配多车位
    // 获取距离并判断车位状态
    int distance = sensor->getDistance();
    if (distance >= 140 && distance <= 280) {
        occupyStatus = 1;
    } else {
        occupyStatus = 0;
    }

    if (!CONF_TEST_IGNORE_VL53L0X_FAILED)
        Log.verboseln("[VL53L0X] 距离%dmm", distance);

    // TODO: 这里要适配多车位
    ParkingSpaceStatus parkingSpaceStatus = {
        .id = settings.parkingSpaceList.spaces[0].id,
        .spaceName = settings.parkingSpaceList.spaces[0].spaceName,
        .occupyStatus = occupyStatus,
        .reservationStatus = reservationStatus,
        .distance = distance
    };
    Log.verboseln("[VL53L0X] 当前状态：%s", fromJsonStruct(parkingSpaceStatus).c_str());

    // 组装消息并发布到MQTT服务器
    if (millis() - lastPublishTime > publishInterval || 1) {
        lastPublishTime = millis();
        String message = fromJsonStruct(parkingSpaceStatus);
        mqttHandler->publishMessage(message.c_str());
    }

    // // 保持MQTT心跳
    // mqttHandler->loop();

    // 检查AT消息
    atHandler->loop();
    
    delay(3000);
}

/**
 * MQTT收到信息后的回调函数
 */
void callbackMqttByPayload(char *topic, byte *payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0'; // 添加字符串结束符
    callbackMqtt(topic, message);
}

/**
 * MQTT收到信息后的回调函数
 */
void callbackMqtt(const char* topic, const char* message) {
    Log.verboseln("[MQTT] 接收到信息：[%s]%s (%u Bytes)", topic, message, strlen(message));

    // 初步处理数据
    JsonDocument doc;                                           // Json数据
    DeserializationError error = deserializeJson(doc, message); // 反序列化Json

    if (error) {
        Log.errorln("[MQTT] Json数据解析失败：%s。", error.f_str());
        return;
    }

    if (doc["code"] != 200) {
        Log.errorln("[MQTT] 服务器返回错误信息：%s。", doc["msg"]);
        return;
    }

    // 提取Data部分
    JsonObject data = doc["data"];
    const char* operation = data["operation"];
    OperationCode code = getOperationCode(operation);

    switch (code) {
    case OPERATION_CONFIGURATION:
        Log.noticeln("[MQTT] 服务器要求配置设备。");
        saveServerConfig(data);
        break;
    case OPERATION_SYNIC_TIME:
        Log.noticeln("[MQTT] 服务器要求同步时间。");
        break;
    case OPERATION_REBOOT:
        Log.noticeln("[MQTT] 服务器要求重启设备。");
        Log.noticeln("[System] 3秒后重启设备。");
        delay(3000);
        ESP.restart();
        break;
    case OPERATION_CHECK_PARKING_STATUS:
        Log.noticeln("[MQTT] 服务器要求上报车位状态。");
        //   mqttHandler->publishMessage(getStatusJson());
        break;
    case OPERATION_CALIBRATE_SENSOR:
        Log.noticeln("[MQTT] 服务器要求进行传感器校准。");
        //   if (!VL53L0XSensor.calibrateSensor()) {
        //     Log.errorln("[VL53L0X] 校准失败！");
        //   }
        break;
    case OPERATION_UPDATE_FIRMWARE:
        Log.noticeln("[MQTT] 服务器要求更新固件。");
        break;
        // 其他操作处理...
    default:
        Log.noticeln("[MQTT] 服务器发出了一个未知指令。");
        break;
    }
}