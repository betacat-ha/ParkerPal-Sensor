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

//=========================调试设置============================
constexpr bool TestExit = false; //硬件测试完成后是否退出

//调试开关
bool Test = true;
const bool TEST_IGNORE_VL53L0X_FAILED = true;

//=======================物联网部分============================
const char *WIFI_SSID = CONF_WIFI_SSID;         // Wi-Fi接入点的名字，最多可以包含32个字符
const char *WIFI_PASSWORD = CONF_WIFI_PASSWORD; // Wi-Fi接入点的密码，长度至少应为8个字符且不超过64个字符
// const char * WIFI_CHANNEL                       // Wi-Fi接入点的频道，可选
// const char * WIFI_BSSID                         // Wi-Fi接入点的MAC地址，可选
WiFiHandler *wifiHandler = nullptr;

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

String spaceId = "1";
String spaceName = "A-035"; // 车位名字
int occupyStatus = 0;       // 占用状态，0表示未占用；1表示被占用；
int reservationStatus = 0;  // 预约状态，0表示未被预约；1表示已被预约

void setup() {
    //=====================初始化串口==============================
    Serial.begin(115200);
    delay(1000);

    // 出现此消息代表串口通信正常
    Serial.print("\n\n");
    Serial.print("================================\n");
    Serial.print("          智泊无忧传感器          \n");
    Serial.print("================================\n");

    //=====================初始化日志框架==========================
    initLog();


    //=====================初始化通信=============================
    // TODO 缺少重试
    wifiHandler = new WiFiHandler(WIFI_SSID, WIFI_PASSWORD);

    if (wifiHandler->connect()) {
        // 如果Wi-Fi连接成功，则初始化MQTTHandler
        mqttHandler = new MQTTHandler(MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT,
                                      MQTT_SERVER_USER, MQTT_SERVER_PASSWORD);
        mqttHandler->setCallback(callback);
        mqttHandler->setBufferSize(5120);

        if (mqttHandler->connect()) {
            mqttHandler->subscribeTopic();
        } else {
            Log.errorln("[MQTT] 无法建立与服务器的连接。");
            // 如果MQTT连接失败，进入死循环
            while (1) {
                system_soft_wdt_feed(); // 喂狗，防止复位
            }
        }
    } else {
        Log.errorln("[Wi-Fi] 无法建立与AP的连接。");
        // 如果Wi-Fi连接失败，进入死循环
        while (1) {
            system_soft_wdt_feed(); // 喂狗，防止复位
        }
    }

    //=====================初始化激光传感器========================
    sensor = new VL53L0XSensor();
    if (!sensor) {
        Log.errorln("[VL53L0X] 初始化失败！");
        while (1 && !TEST_IGNORE_VL53L0X_FAILED) {
            system_soft_wdt_feed(); // 喂狗，防止复位
        }
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

    WiFiScanList list = wifiHandler->scanNetworks();
    String listJson = fromJsonStruct(list);
    Log.verboseln("[Wi-Fi] AP列表：%s", listJson.c_str());
    mqttHandler->publishMessage(listJson.c_str());

    // 获取距离并判断车位状态
    int distance = sensor->getDistance();
    if (distance >= 140 && distance <= 280) {
        occupyStatus = 1;
    } else {
        occupyStatus = 0;
    }
    
    if (!TEST_IGNORE_VL53L0X_FAILED) 
        Log.verboseln("[VL53L0X] 距离%dmm", distance);

    ParkingSpaceStatus parkingSpaceStatus = {
        .id = spaceId,
        .spaceName = spaceName,
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

    // 保持MQTT心跳
    mqttHandler->loop();
    delay(3000);
}

/**
* MQTT收到信息后的回调函数
*/
void callback(char* topic, byte* payload, unsigned int length) {
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';  // 添加字符串结束符

  Log.verboseln("[MQTT] 接收到信息：[%s]%s (%u Bytes)", topic, message, length);

  // 初步处理数据
  JsonDocument doc;                                            // Json数据
  DeserializationError error = deserializeJson(doc, message);  // 反序列化Json

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
    case OPERATION_MODIFY_SETTINGS:
      Log.noticeln("[MQTT] 服务器要求更新配置文件。");
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
    // 其他操作处理...
    default:
      Log.noticeln("[MQTT] 服务器发出了一个未知指令。");
      break;
  }
}