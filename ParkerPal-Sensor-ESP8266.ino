/*项目名称：智泊无忧传感器端
  开发团队：智泊无忧硬件组
  立项时间：2024.8.30
*/
#include <SPI.h>
#include <Wire.h>
#include <ArduinoLog.h>
#include "wifi_handler.hpp"
#include "mqtt_handler.hpp"
#include "log_helper.hpp"
#include "json_helper.hpp"
#include "vl53l0x_sensor.hpp"
#include "operations.hpp"

//=======================基础设置==========================
constexpr bool TestExit = false;  //硬件测试完成后是否退出

//调试开关
bool Test = true;

// 串口通信部分
int SerialData = 0;  //串口传入数据
String comdata = "";

//=========================接口============================


//=======================物联网部分============================
const char * WIFI_SSID  = "mahaka_iot";            // Wi-Fi接入点的名字，最多可以包含32个字符
const char * WIFI_PASSWORD  = "WDAd1a31d3aWA&&#";  // Wi-Fi接入点的密码，长度至少应为8个字符且不超过64个字符
// const char * WIFI_CHANNEL                       // Wi-Fi接入点的频道，可选
// const char * WIFI_BSSID                         // Wi-Fi接入点的MAC地址，可选
WiFiHandler * wifiHandler = nullptr;

const char * MQTT_SERVER_ADDRESS = "broker.emqx.io";
// const char * MQTT_SERVER_ADDRESS = "192.168.100.150";      // MQTT服务器地址
const char * MQTT_SERVER_USER = "ParkerPal";               // MQTT服务器用户
const char * MQTT_SERVER_PASSWORD = "123456";              // MQTT服务器密码
constexpr uint16_t MQTT_SERVER_PORT = 1883;
MQTTHandler * mqttHandler = nullptr;



//=======================车位检测部分============================
VL53L0XSensor * sensor = nullptr;

// 车位占用判断阈值
constexpr int OCCUPY_THRESHOLD_MIN = 140;  // 判断车位占用的最低界限
constexpr int OCCUPY_THRESHOLD_MAX = 280;  // 判断车位占用的最高界限

String spaceName = "A-035";               // 车位名字
int occupyStatus = 0;                     // 占用状态，0表示未占用；1表示被占用；
int reservationStatus = 0;                // 预约状态，0表示未被预约；1表示已被预约


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
      mqttHandler = new MQTTHandler(MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT, MQTT_SERVER_USER, MQTT_SERVER_PASSWORD);
      // mqttHandler->setCallback(callback);

      if (mqttHandler->connect()) {
          mqttHandler->subscribeTopic("inTopic");
      } else {
          Log.errorln("Failed to establish MQTT connection.");
          while (1); // 如果MQTT连接失败，进入死循环
      }
  } else {
      Log.errorln("Failed to establish Wi-Fi connection.");
      while (1); // 如果Wi-Fi连接失败，进入死循环
  }

  //=====================初始化激光传感器========================
  // sensor = new VL53L0XSensor();
  // if (!sensor) {
  //     Log.errorln("[VL53L0X] Unable to start laser distance sensor!");
  //     while (1);
  // }
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
    // int distance = sensor.getDistance();
    // if (distance >= 140 && distance <= 280) {
    //     occupyStatus = 1;
    // }

    // wifiHandler->scanAndPrintNetworks(); // TODO 能正确扫描，下一步把信息存入结构体转换成json数据

    ParkingSpaceStatus a = {
      .spaceName = "",
      .occupyStatus = 0,
      .reservationStatus = 0,
      .distance = 0.0f
    };

    Log.noticeln(fromJsonStruct(a).c_str());

    // // 组装消息并发布到MQTT服务器
    // if (millis() - lastPublishTime > publishInterval) {
    //     lastPublishTime = millis();
    //     String message = assembleMQTTMessage("A-035", occupyStatus, 0);
    //     mqttHandler->publishMessage(WiFi.macAddress().c_str(), message.c_str());
    // }

<<<<<<< HEAD
    // // 保持MQTT心跳
    // mqttHandler->loop();
=======
  if (mqttClient.connected()) {  // 如果开发板成功连接服务器
    //发布信息
    // pubMQTTmsg();
    // 保持心跳
    mqttClient.loop();
  } else {                // 如果开发板未能成功连接服务器
    connectMQTTServer();  // 则尝试连接服务器
  }

  // 每隔3秒发送一次
  // delay(500);

  /*
    ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
    ┃                                                            ┃
    ┃                                                            ┃
    ┃                       Log Output                           ┃
    ┃                                                            ┃
    ┃                                                            ┃
    ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  */

  if (Test == true) {
    if (lox.begin()) {
      lox.rangingTest(&measure, false);  // pass in 'true' to get debug data printout!
      Log.verboseln("[VL53L0X] 当前与物体的距离(mm):%d", measure.RangeMilliMeter);
    } else {
      Log.errorln("[VL53L0X] 无法启动VL53L0X激光测距仪");
    }
    delay(500);
  }
}

// 激光传感器校准
int calibrateSensor() {
  Log.notice("[VL53L0X] 校准流程开始！\n当前数值:%d \n请使用物品完全覆盖住传感器，并保持不动，直到校准完成。\n3秒后校准流程自动开始！" CR, Distance_calibration);

  delay(3000);

  int result = 0;
  int times = 0;
  int SuccessfulTimes = 0;

  //验证机制，防止接线松动
  while (times <= 10 & SuccessfulTimes <= 3) {

    if (lox.begin() & measure.RangeStatus != 4) {
      lox.rangingTest(&measure, false);

      //算出平均值，减少误差
      if (result == 0)
        result = int(measure.RangeMilliMeter);
      else if (result != 0)
        result = (int(measure.RangeMilliMeter) + result) / 2;

      SuccessfulTimes++;
    }

    times++;
  }


  if (result != 0) {
    //存储原始数据
    Distance_calibration = result;
    Log.noticeln("[VL53L0X] 校准完成！");
    return 0;
  } else {
    Log.errorln("[VL53L0X] 校准失败！");
    return -1;
  }
}

void getOccupyStatus() {
  return;
}

int getAvgDistance() {
  int result = 0;
  int times = 0;
  int SuccessfulTimes = 0;

  //验证机制，防止接线松动
  while (times <= 10 & SuccessfulTimes <= 3) {

    if (lox.begin() & measure.RangeStatus != 4) {

      lox.rangingTest(&measure, false);

      //算出平均值，减少误差
      if (result == 0)
        result = int(measure.RangeMilliMeter);
      else if (result != 0)
        result = (int(measure.RangeMilliMeter) + result) / 2;

      SuccessfulTimes++;
    }

    times++;
  }

  // 如果没有结果，返回-1，如果有结果，返回校准后的数据
  return result == 0 ? -1 : result;
}

int getSingleDistance() {
  if (lox.begin()) {
    lox.rangingTest(&measure, false);
    return measure.RangeMilliMeter;
  }

  // 未知异常返回-1
  return -1;
}

//倒数计时
void delaytime(int delaytime) {
  Serial.println("");
  while (delaytime != 0) {
    // Serial.print(delaytime);
    delaytime--;
    delay(1000);  //等待1000毫秒
  }
  delaytime = 0;
  // Serial.println("");
}

/**
* 连接到Wi-Fi
*/
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // 轮询检查是否连接成功
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    i++;
    delay(1000);
    if (i > 15) {  // 15秒后如果还是连接不上，就判定为连接超时
      Log.errorln("[Wi-Fi] 连接失败：%s" CR, localizableWLStatus(WiFi.status()));
      delay(1000);
      break;
    }
    Log.verbose("[Wi-Fi] 等待连接，当前状态码：%d" CR, WiFi.status());
  }
  Log.notice("[Wi-Fi] 当前状态：%s" CR, localizableWLStatus(WiFi.status()));
}

/**
* 连接到MQTTServer
*/
void connectMQTTServer() {
  // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
  String clientId = "esp8266-" + WiFi.macAddress();
  Log.notice("[MQTT] 开始连接到Server..." CR);
  Log.noticeln("[MQTT] Client ID: %s", clientId.c_str());
  Log.noticeln("[MQTT] Server Address: %s", MQTT_SERVER_ADDRESS);

  // 设置MQTT服务器和端口号
  mqttClient.setServer(MQTT_SERVER_ADDRESS, 1883);
  // 设置MQTT订阅回调函数
  mqttClient.setCallback(receiveMQTTCallback);

  // 轮询检查是否连接成功
  int i = 0;
  while (!mqttClient.connect(clientId.c_str())) {
    i++;
    delay(3000);
    if (i > 3) {  // 重试3次，如果还是连接不上，就判定为连接超时
      Log.errorln("[MQTT] 连接失败：状态码：%d" CR, mqttClient.state());
      delay(1000);
      break;
    }
    Log.verboseln("[MQTT] 等待连接，状态码：%d", mqttClient.state());
  }
  Log.noticeln("[MQTT] 当前状态：%d", mqttClient.state());
}

/**
 * 发布信息到MQTT服务器的默认主题
 * @param message 发布的消息内容
 */
void pubMQTTmsg(const String& message) {
  // 建立发布主题。主题名称以Sensor-为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同用户进行MQTT信息发布时，ESP8266客户端名称各不相同，
  String topicString = "Sensor-Pub-" + WiFi.macAddress();
  char publishTopic[topicString.length() + 1];
  strcpy(publishTopic, topicString.c_str());

  pubMQTTmsg(topicString, message);
}

/**
 * 发布信息到 MQTT 服务器
 * @param topic 发布的主题
 * @param message 发布的消息内容
 */
void pubMQTTmsg(const String& topic, const String& message) {
  // 将主题和消息转换为 C 风格字符串
  char publishTopic[topic.length() + 1];
  strcpy(publishTopic, topic.c_str());

  char publishMsg[message.length() + 1];
  strcpy(publishMsg, message.c_str());

  // 发布消息到 MQTT 服务器
  if (mqttClient.publish(publishTopic, publishMsg)) {
    Log.verboseln("[MQTT] 已向 Server 发布信息，主题：%s，信息：%s", publishTopic, publishMsg);
  } else {
    Log.errorln("[MQTT] 信息发布失败。");
  }
}

/**
 * 将 JSON 字符串解析为 JsonObject
 * @param jsonString 输入的 JSON 字符串
 * @return 返回解析得到的 JsonObject，解析失败时返回包含占位信息的 JsonObject
 */
JsonObject stringToJsonObject(const String& jsonString) {
    // 必须创建静态对象，否则函数结束，所分配的内存就会被销毁
    static StaticJsonDocument<256> doc;

    // 清空文档，确保内容不会混淆
    doc.clear();

    // 尝试解析 JSON 字符串
    DeserializationError error = deserializeJson(doc, jsonString);

    // 如果解析失败，返回带有占位信息的对象
    if (error) {
        JsonObject placeholder = doc.to<JsonObject>();
        placeholder["error"] = "Invalid JSON";
        return placeholder;
    }

    // 返回解析成功的 JsonObject
    return doc.as<JsonObject>();
}

/**
 * 组装发布VL53L0X状态Json
 * @return 返回组装好的消息字符串
 */
String getLoxStatusJson() {
  JsonDocument doc;
  doc["sensorType"] = "VL53L0X";
  doc["spaceName"] = spaceName;
  doc["occupyStatus"] = occupyStatus;
  doc["reservationStatus"] = reservationStatus;

  String output;
  serializeJson(doc, output);

  return output;
}

/**
 * 组装发布所有状态Json
 * @return 返回组装好的消息字符串
 */
String getStatusJson() {
  JsonDocument doc;
  doc["uuid"] = "123456789";
  doc["powerMode"] = "AC";
  doc["batteryLife"] = 100.0;

  // 添加激光传感器的状态数据
  doc["sensorStatus"][0] = stringToJsonObject(getLoxStatusJson());

  String output;
  serializeJson(doc, output);

  return output;
}

/**
* 订阅MQTT指定主题
*/
void subscribeTopic() {

  // 建立订阅主题。主题名称以Sensor-为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同设备使用同一个MQTT服务器测试消息订阅时，所订阅的主题名称不同
  String topicString = "Sensor-Sub-" + WiFi.macAddress();
  char subTopic[topicString.length() + 1];
  strcpy(subTopic, topicString.c_str());

  // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
  if (mqttClient.subscribe(subTopic)) {
    Log.verboseln("[MQTT] 已订阅主题：%s", subTopic);
  } else {
    Log.errorln("[MQTT] 主题订阅失败。");
  }
}

/**
* MQTT收到信息后的回调函数
*/
void receiveMQTTCallback(char* topic, byte* payload, unsigned int length) {
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
    case OPERATION_CHECK_STATUS:
      Log.noticeln("[MQTT] 服务器要求上报状态。");
      pubMQTTmsg(getStatusJson());
      break;
    case OPERATION_CALIBRATE_SENSOR:
      Log.noticeln("[MQTT] 服务器要求进行传感器校准。");
      if (!calibrateSensor()) {
        Log.errorln("[VL53L0X] 校准失败！");
      }
      break;
    // 其他操作处理...
    default:
      Log.noticeln("[MQTT] 服务器发出了一个未知指令。");
      break;
  }
>>>>>>> 1f70836640ed418136d7998766dba692c6948ea2
}
