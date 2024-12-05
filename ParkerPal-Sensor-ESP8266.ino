/*项目名称：智泊无忧传感器端
  开发团队：智泊无忧硬件组
  立项时间：2024.8.30
*/
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_VL53L0X.h"  // 激光传感器库
#include <ESP8266WiFi.h>       // Wi-Fi库
#include <ArduinoLog.h>        // Log日志框架
#include <PubSubClient.h>      // MQTT库
#include <ArduinoJson.h>       // Json库
#include "operations.h"

//=======================基础设置==========================
const bool TestExit = false;  //硬件测试完成后是否退出

//调试开关
bool Test = true;

// 串口通信部分
int SerialData = 0;  //串口传入数据
String comdata = "";

//=========================接口============================


//=======================物联网部分============================
// Wi-Fi信息
#define WIFI_SSID "mahaka_iot"            // Wi-Fi接入点的名字，最多可以包含32个字符
#define WIFI_PASSWORD "WDAd1a31d3aWA&&#"  // Wi-Fi接入点的密码，长度至少应为8个字符且不超过64个字符
// #define WIFI_CHANNEL                      // Wi-Fi接入点的频道，可选
// #define WIFI_BSSID                        // Wi-Fi接入点的MAC地址，可选
#define WIFI_CONNECT true  // 开机自动连接

#define MQTT_SERVER_ADDRESS "192.168.100.150"  // MQTT服务器地址
#define MQTT_SERVER_PASSWORD ""                // MQTT服务器密码（未启用）
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);



//=======================车位检测部分============================

// 激光测距仪部分
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int Distance_calibration = 30;  // 激光测距仪校准数据（低于此值的数据直接丢弃）
int distance = 0;               // 激光测距仪数据
VL53L0X_RangingMeasurementData_t measure;

// 车位信息
int occupyStatus = 0;             // 占用状态，0表示未占用；1表示被占用；
int reservationStatus = 0;        // 预约状态，0表示未被预约；1表示已被预约
String spaceName = "A-035";       // 车位名字
#define OCCUPY_THRESHOLD_MAX 280  // 判断车位占用的最高界限
#define OCCUPY_THRESHOLD_MIN 140  // 判断车位占用的最低界限


void setup() {
  //=====================初始化串口==============================
  Serial.begin(115200);
  delay(100);
  Serial.print("\n\n");
  Serial.print("================================\n");
  Serial.print("          智泊无忧传感器          \n");
  Serial.print("================================\n");


  //=====================初始化日志框架==========================
  Log.setPrefix(printPrefix);  // 设置前缀
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice("[Log] 开始初始化..." CR);


  //=====================初始化通信=============================
  // 连接Wi-Fi
  Log.notice("[Wi-Fi] 开始连接到AP..." CR);
  connectWiFi();


  // 初始化MQTT组件
  connectMQTTServer();
  subscribeTopic();

  //=====================初始化激光传感器========================
  if (!lox.begin()) {
    Log.noticeln("[VL53L0X] 无法启动激光测距仪！");
    Log.noticeln("[System] 启动中止，请重置开发板");
    while (1) {
      system_soft_wdt_feed();  // 喂狗，防止复位
    }
  }

  // 清空脏数据
  lox.rangingTest(&measure, false);
  delay(500);
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

  if (Serial.available() > 0) {
    //获取串口接收到的数据
    SerialData = Serial.read();

    /************************距离传感器校准*********************************/
    if (SerialData == 'C') {
      calibrateSensor();
    }
  }

  /************************获取物体距离*********************************/

  int singleDistance = getSingleDistance();

  // 如果数值大于校准值，说明值有效，则写入到结果中
  if (singleDistance > Distance_calibration) {
    distance = singleDistance - Distance_calibration;
  }

  // 如果距离处于占用阈值内，则判断为被占用
  if (distance >= OCCUPY_THRESHOLD_MIN && distance <= OCCUPY_THRESHOLD_MAX) {
    occupyStatus = 1;
  } else {
    occupyStatus = 0;
  }

  if (mqttClient.connected()) {  // 如果开发板成功连接服务器
    //发布信息
    // pubMQTTmsg();
    // 保持心跳
    mqttClient.loop();
  } else {                // 如果开发板未能成功连接服务器
    connectMQTTServer();  // 则尝试连接服务器
  }

  // 每隔3秒发送一次
  delay(3000);

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

  pubMQTTmsg(topicString, message)
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
 * 组装发布的消息内容
 * @param spaceName 车位名称
 * @param occupyStatus 车位占用状态 (0: 空闲, 1: 占用)
 * @param reservationStatus 预约状态 (0: 未预约, 1: 已预约)
 * @return 返回组装好的消息字符串
 */
String assembleMQTTMessage(const String& spaceName, int occupyStatus, int reservationStatus) {
  return "车位名:" + spaceName + " | 占用状态:" + String(occupyStatus) + " | 预约状态:" + String(reservationStatus);
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
      pubMQTTmsg();
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
}
