#include <SPI.h>
#include <SD.h>

// sound sensor and SD card CS pin
const int soundSensorPin = A0; //sound sensor analog pin 0
const int chipSelect = 4; // sd card digital pin 4
File dataFile;


// push buttom
const int buttonPin = 2; // pin digital pin 2
int lastButtonState = HIGH; 

//vibration motor
const int vibrationMotorPin = 5; // digital pin 5
int vibrationMode = 1; //0 close and 1 is open

//buzzer 
const int buzzerPin = 3;// buzzer digital pin 3
int buzzerMode = 0;// buzzer close 0, correct 1, incorrect 2


void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);// log on the pin for push buttom
  pinMode(vibrationMotorPin, OUTPUT); // log on the pin for vibration motor
  pinMode(buzzerPin, OUTPUT);// log on the pin for buzzer
  
  if (!SD.begin(chipSelect)) {
    Serial.println("SD卡初始化失败");
    while (1); // if no SD card not going to next step
  }
  Serial.println("SD卡初始化成功");
  
  // creat open the txt file
  dataFile = SD.open("sound.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println("打开文件失败");
  }
  
}

void loop() {
  // 读取按钮状态
  int buttonState = digitalRead(buttonPin);
  // 检测按钮的状态变化
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) { // 如果按钮被按下
      Serial.println("Button Pressed"); // 显示触发状态
    }
    lastButtonState = buttonState; // 更新按钮状态
    delay(50); // 简单的防抖动延迟
  }
  //read data from other Ardurino
  if (Serial1.available() > 0) {
    String uid = Serial1.readStringUntil('\n'); // 读取一行数据
    Serial1.print("接收到UID: ");
    Serial1.println(uid);
    SerialUSB.println(uid);
  }
  
  // 持续读取声音传感器并记录数据
  if (dataFile) {
    int sensorValue = analogRead(soundSensorPin);
    dataFile.println(sensorValue);
    Serial.println(sensorValue);
  } else {
    Serial.println("数据文件未打开，无法写入数据");
  }

  // 每次记录后短暂延时，减少数据点数量
  delay(100); // 根据需要调整延时时长
  
  if (vibrationMode !=0){
    digitalWrite(vibrationMotorPin, HIGH); // 开启震动
    delay(300); // 震动1秒
    digitalWrite(vibrationMotorPin, LOW); // 关闭震动
    delay(300); // 停止1秒
    }
  if (buzzerMode !=0){
    tone(buzzerPin, 1000); // 在引脚上生成1000Hz的音调
    delay(1000); // 持续1秒
    noTone(buzzerPin); // 停止发声
    delay(1000); // 等待1秒
  }else if(buzzerMode !=1){
    tone(buzzerPin, 1000); // 在引脚上生成1000Hz的音调
    delay(500); // 持续1秒
    noTone(buzzerPin); // 停止发声
    delay(500); // 等待1秒
  }else if(buzzerMode !=2){
    tone(buzzerPin, 500); // 在引脚上生成1000Hz的音调
    delay(500); // 持续1秒
    noTone(buzzerPin); // 停止发声
    delay(500); // 等待1秒
  }
  delay(1000);
}
// 在结束时关闭文件
void endRecording() {
  if (dataFile) {
    dataFile.close();
    Serial.println("录音结束，文件已保存。");
  }
}
