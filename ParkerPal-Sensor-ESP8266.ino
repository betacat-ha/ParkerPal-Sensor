/*项目名称：智泊无忧传感器端
  开发团队：智泊无忧硬件组
  立项时间：2024.8.30
*/
#include "json_helper.hpp"
#include "log_helper.hpp"
#include "mqtt_handler.hpp"
#include "operations.hpp"
#include "vl53l0x_sensor.hpp"
#include "wifi_handler.hpp"
#include <ArduinoLog.h>
#include <SPI.h>
#include <Wire.h>

//=======================基础设置==========================
constexpr bool TestExit = false; //硬件测试完成后是否退出

//调试开关
bool Test = true;

// 串口通信部分
int SerialData = 0; //串口传入数据
String comdata = "";

//=========================接口============================

//=======================物联网部分============================
const char *WIFI_SSID = "mahaka_iot"; // Wi-Fi接入点的名字，最多可以包含32个字符
const char *WIFI_PASSWORD =
    "WDAd1a31d3aWA&&#"; // Wi-Fi接入点的密码，长度至少应为8个字符且不超过64个字符
// const char * WIFI_CHANNEL                       // Wi-Fi接入点的频道，可选
// const char * WIFI_BSSID                         // Wi-Fi接入点的MAC地址，可选
WiFiHandler *wifiHandler = nullptr;

const char *MQTT_SERVER_ADDRESS = "broker.emqx.io";
// const char * MQTT_SERVER_ADDRESS = "192.168.100.150";      // MQTT服务器地址
const char *MQTT_SERVER_USER = "ParkerPal";  // MQTT服务器用户
const char *MQTT_SERVER_PASSWORD = "123456"; // MQTT服务器密码
constexpr uint16_t MQTT_SERVER_PORT = 1883;
MQTTHandler *mqttHandler = nullptr;

//=======================车位检测部分============================
VL53L0XSensor *sensor = nullptr;

// 车位占用判断阈值
constexpr int OCCUPY_THRESHOLD_MIN = 140; // 判断车位占用的最低界限
constexpr int OCCUPY_THRESHOLD_MAX = 280; // 判断车位占用的最高界限

String spaceName = "A-035"; // 车位名字
int occupyStatus = 0; // 占用状态，0表示未占用；1表示被占用；
int reservationStatus = 0; // 预约状态，0表示未被预约；1表示已被预约

void setup() {
    //=====================初始化串口==============================
    Serial.begin(115200);
    delay(1000);

    //=====================初始化日志框架==========================
    initLog();

    Serial.print("\n\n");
    Serial.print("================================\n");
    Serial.print("          智泊无忧传感器          \n");
    Serial.print("================================\n");

    //=====================初始化通信=============================
    // TODO 缺少重试
    wifiHandler = new WiFiHandler(WIFI_SSID, WIFI_PASSWORD);

    if (wifiHandler->connect()) {
        // 如果Wi-Fi连接成功，则初始化MQTTHandler
        mqttHandler = new MQTTHandler(MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT,
                                      MQTT_SERVER_USER, MQTT_SERVER_PASSWORD);
        // mqttHandler->setCallback(callback);

        if (mqttHandler->connect()) {
            mqttHandler->subscribeTopic("inTopic");
        } else {
            Log.errorln("Failed to establish MQTT connection.");
            // 如果MQTT连接失败，进入死循环
            while (1) {
                system_soft_wdt_feed(); // 喂狗，防止复位
            }
        }
    } else {
        Log.errorln("Failed to establish Wi-Fi connection.");
        // 如果Wi-Fi连接失败，进入死循环
        while (1) {
            system_soft_wdt_feed(); // 喂狗，防止复位
        }
    }

    //=====================初始化激光传感器========================
    sensor = new VL53L0XSensor();
    if (!sensor) {
        Log.errorln("[VL53L0X] 初始化失败！");
        while (1);
    }
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

    // 获取距离并判断车位状态
    // int distance = sensor->getDistance();
    // if (distance >= 140 && distance <= 280) {
    //     occupyStatus = 1;
    // }

    // wifiHandler->scanAndPrintNetworks(); 

    WiFiScanList list = wifiHandler->scanNetworks();
    Log.noticeln(fromJsonStruct(list).c_str());
    delay(3000);

    // ParkingSpaceStatus a = {
    //     .spaceName = "",
    //     .occupyStatus = 0,
    //     .reservationStatus = 0,
    //     .distance = 0.0f
    // };
    // Log.noticeln(fromJsonStruct(a).c_str());

    // // 组装消息并发布到MQTT服务器
    // if (millis() - lastPublishTime > publishInterval) {
    //     lastPublishTime = millis();
    //     String message = assembleMQTTMessage("A-035", occupyStatus, 0);
    //     mqttHandler->publishMessage(WiFi.macAddress().c_str(),
    //     message.c_str());
    // }

    // // 保持MQTT心跳
    // mqttHandler->loop();
}
