/*项目名称：智泊无忧传感器端
  开发团队：智泊无忧硬件组
  立项时间：2024.8.30
*/
#include <SPI.h>

#include <Wire.h>
#include "Adafruit_VL53L0X.h"
#include <ESP8266WiFi.h>
#include <ArduinoLog.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#endif

//=======================基础设置==========================
const bool TestExit = false;  //硬件测试完成后是否退出

//调试开关
bool Test = true;

// 串口通信部分
int SerialData = 0;  //串口传入数据
String comdata = "";

//=========================接口============================
// LCD屏
const int LCD_SCK = D5;  // 时钟
const int LCD_SDA = D6;  // 数据
const int LCD_RST = D4;  // 复位
const int LCD_CD = D7;   // 数据/命令
const int LCD_CS = D8;   // 片选

// LCD显示
U8G2_UC1604_JLX19264_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/LCD_SCK, /* data=*/LCD_SDA, /* cs=*/LCD_CS, /* dc=*/LCD_CD, /* reset=*/LCD_RST);

//=======================物联网部分============================
// Wi-Fi信息
#define WIFI_SSID "mahaka_iot"                // Wi-Fi接入点的名字，最多可以包含32个字符
#define WIFI_PASSWORD "WDAd1a31d3aWA&&#"  // Wi-Fi接入点的密码，长度至少应为8个字符且不超过64个字符
// #define WIFI_CHANNEL                      // Wi-Fi接入点的频道，可选
// #define WIFI_BSSID                        // Wi-Fi接入点的MAC地址，可选
#define WIFI_CONNECT true  // 开机自动连接


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
  
  Log.setPrefix(printPrefix); // 设置前缀
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice("开始初始化..." CR);


  //=====================初始化LCD驱动===========================
  initLCD();
  displayLogo();  // 显示Logo
  delay(3000);


  //=====================初始化Wi-Fi=============================
  // 连接Wi-Fi
  displayAPConfig(true, "正在连接到AP...");
  Log.notice("开始连接到AP..." CR);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // 轮询检查是否连接成功
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    i++;
    delay(1000);
    if (i > 15) {  // 15秒后如果还是连接不上，就判定为连接超时
      Log.errorln("连接失败：%s" CR, localizableWLStatus(WiFi.status()));
      displayAPConfig(true, "连接失败，进入本地模式");
      delay(1000);
      break;
    }
    Log.verbose("等待连接，当前Wi-Fi状态码：%d" CR, WiFi.status());
  }
  Log.notice("当前Wi-Fi状态：%s" CR, localizableWLStatus(WiFi.status()));


  //=====================初始化激光传感器===========================
  u8g2.clear();
  u8g2.setCursor(10, 15);
  u8g2.println("激光测距");
  u8g2.setCursor(10, 35);
  u8g2.println("正在自检...");
  u8g2.sendBuffer();

  if (!lox.begin()) {
    u8g2.clear();
    u8g2.setCursor(10, 15);
    u8g2.println("激光测距");
    u8g2.setCursor(10, 35);
    u8g2.println("无法启动激光测距仪");
    u8g2.setCursor(10, 55);
    u8g2.println("启动中止，请重置开发板");
    u8g2.sendBuffer();
    while (1) {
      system_soft_wdt_feed(); // 喂狗，防止复位
    }
  }

  // 清空脏数据
  lox.rangingTest(&measure, false);
  delay(500);

  displayLogo();
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
      Log.notice("校准流程开始！\n当前数值:%d \n请使用物品完全覆盖住传感器，并保持不动，直到校准完成。\n3秒后校准流程自动开始！" CR, Distance_calibration);

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
        Log.noticeln("校准完成！");
      } else {
        Log.errorln("校准失败！");
      }
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

  displaySensorInfo();

  delay(500);

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
      Log.verboseln("当前与物体的距离(mm):%d", measure.RangeMilliMeter);
    } else {
      Log.errorln("无法启动VL53L0X激光测距仪");
    }
    delay(500);
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


void initLCD() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_wqy16_t_gb2312);
  u8g2.enableUTF8Print();
  u8g2.sendF("c", 0xeb);  //设置LCD偏置比(亮度设置)
  u8g2.sendF("c", 0x81);  //设置SEG偏置电压(对比度)
  u8g2.sendF("c", 0xa3);  //设置帧速率
  u8g2.sendF("c", 0x2f);  //显示屏功耗设置
}

void displaySensorInfo() {
  u8g2.clear();

  u8g2.setCursor(0, 15);
  u8g2.println("车位名");
  u8g2.setCursor(75, 15);
  u8g2.println(spaceName);

  u8g2.setCursor(0, 35);
  u8g2.println("距离(mm)");
  u8g2.setCursor(75, 35);
  u8g2.println(distance);
  if (distance >= 8160)
    u8g2.println(" (超限)");
  if (distance < 10)
    u8g2.println(" (清洁传感器)");

  u8g2.setCursor(0, 55);
  u8g2.println("车位状态");
  u8g2.setCursor(75, 55);
  u8g2.println(reservationStatus == 1 ? "有预约" : "未预约");
  u8g2.println(" | ");
  u8g2.println(occupyStatus == 1 ? "已占用" : "未占用");

  u8g2.sendBuffer();
}

void displayAPConfig(bool clear, String note) {
  if (clear)
    u8g2.clear();

  u8g2.setCursor(10, 15);
  u8g2.print("SSID: ");
  u8g2.println(WIFI_SSID);
  u8g2.setCursor(10, 35);
  u8g2.print("PWD: ");
  u8g2.println(WIFI_PASSWORD);
  u8g2.setCursor(10, 55);
  u8g2.println(note);
  u8g2.sendBuffer();
}

void displayLogo() {
  u8g2.clear();
  u8g2.setCursor(28, 35);
  u8g2.println("ParkerPal 智泊无忧");
  u8g2.sendBuffer();
}