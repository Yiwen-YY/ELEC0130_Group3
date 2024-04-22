#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>  // 引入ArduinoJson库
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000
#include <SPI.h>
#include <SD.h>
#include "arduino_secrets.h"

// sound sensor and SD card CS pin
const int soundSensorPin = A0; //sound sensor analog pin 0
const int chipSelect = 4; // sd card digital pin 4
File dataFile;

// push buttom
const int buttonPin = 2; // pin digital pin 2
int lastButtonState = LOW; 

//vibration motor
const int vibrationMotorPin = 5; // digital pin 5

//buzzer 
const int buzzerPin = 3;// buzzer digital pin 3
int buzzerMode = 0;// buzzer close 0, correct 1, incorrect 2

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = "羊屁屁";
const char pass[]        = "12345678";
const char broker[]      = "a2bu9nf5celvb1-ats.iot.eu-west-2.amazonaws.com";
const char* certificate  = R"(
-----BEGIN CERTIFICATE-----
MIICgTCCAWmgAwIBAgIUHG+OF0bAW4NW57G1G9FGN1vWicgwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI0MDMwODAwMjcy
MloXDTQ5MTIzMTIzNTk1OVowETEPMA0GA1UEAxMGbXkxMDEwMFkwEwYHKoZIzj0C
AQYIKoZIzj0DAQcDQgAEU0NMDZ5ti9FlMY62nPiOAILNQECbYbeal9VpGkRDsiMK
6SRb3DIlWYFzxdVLrdc7fYIHOT7tOgxh5YLngLziP6NgMF4wHwYDVR0jBBgwFoAU
WiMyK7Tw2PIRZVWmrfJe4wiX5VwwHQYDVR0OBBYEFL42wfaTwfKQaWsvv3MTwIwK
2b4kMAwGA1UdEwEB/wQCMAAwDgYDVR0PAQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUA
A4IBAQBGBQMVa/d9IF7gynJb6AL3XreKifkICoM71owg7Lp44xrKQluU8KwLZV6M
rpmCZwEacoc1iMaufBAJ5FY1Br3+uM6DF1uOUBSe+xwUJXdWo1ThIIlmhTC7989w
DgS/d0quSGBHpqGXRQ1ixB3QtBewOUpnTSOyu++kG5EbIgkCOx5dqySNFK8LzcSL
EvUGhXgCEaI7nDpQlNX14Jq3rVB4UPyi+DvEa+zV92pW+jFO4GhXklJxoKGYWRIV
ZkWgKpLIGJzpfN/lNp5+7rBWgbGoCEUdMC9PaUuoYyqKbBtNuht7xDwL1eI9t/kj
ftEmj/9BgVuuYgcMPlE1cv0y3Iz7
-----END CERTIFICATE-----
)";

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);
int receivedValue = -1; // initialise the reciving value
unsigned long lastMillis = 0; // time

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);// log on the pin for push buttom
  pinMode(vibrationMotorPin, OUTPUT); // log on the pin for vibration motor
  pinMode(buzzerPin, OUTPUT);// log on the pin for buzzer
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Fail to read SD card");
    while (1); // if no SD card not going to next step
  }
  Serial.println("SD card initialised");
  
  // creat open the txt file
  dataFile = SD.open("sound.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println("fail to read file");
  }
  //////////////////////////////////////////////////////////////////////////////////////////////wifi connection
  Serial.begin(115200);
  while (!Serial);

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  sslClient.setEccSlot(0, certificate);

  // Optional, set the client id used for MQTT,
  // each device that is connected to the broker
  // must have a unique client id. The MQTTClient will generate
  // a client id for you based on the millis() value if not set
  //
  // mqttClient.setId("clientId");

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  
}

void loop() {
  mqttClient.onMessage(onMessageReceived);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Wifi and MQTT
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }
  // poll for new MQTT messages and send keep alives
  mqttClient.poll();


  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////sound sensor
  // 持续读取声音传感器并记录数据
  if (dataFile) {
    int sensorValue = analogRead(soundSensorPin);
    dataFile.println(sensorValue);
  } else {
    Serial.println("数据文件未打开，无法写入数据");
  }

  // 每次记录后短暂延时，减少数据点数量
  delay(100); // 根据需要调整延时时长
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////push buttonState
  int buttonState = digitalRead(buttonPin);
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) { // if pushed
      Serial.println("Button Pressed"); // tell it is pushed
    }
    lastButtonState = buttonState; // update
    delay(50);
  }
  
  
  //read data from other Ardurino//////////////////////////////////////////////////////////////////////////////////////////////////RFID
  if (Serial1.available() > 0) {
    String uid = Serial1.readStringUntil('\n'); // read value
    StaticJsonDocument<200> doc;
    doc["UID"] = uid;
    // 将JSON对象序列化为字符串
    String output;
    serializeJson(doc, output);
    Serial1.println(); // 输出JSON格式的字符串
    SerialUSB.println(uid);
    SerialUSB.println(output);
    // publish a message roughly every 5 seconds.
    if (millis() - lastMillis > 2000) {
      lastMillis = millis();
      publishMessage(output);
    }
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module  
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("arduino/incoming");
}

void publishMessage(String output) {
  Serial.println("Publishing UID");
  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage("arduino/outgoing");
  mqttClient.print(output);
  mqttClient.endMessage();
}


void onMessageReceived(int messageSize) {
  // 获取消息的主题
  String topic = mqttClient.messageTopic();
  
  // 检查是否为我们感兴趣的主题
  if(topic == "arduino/incoming") {
    // 准备一个字符串来存储消息内容
    String content = "";
    // 读取整个消息内容
    while (mqttClient.available()) {
      char c = (char)mqttClient.read();
      content += c;
    }
    
    //////////////////////////////////////////////////////////////////////////////////// if the incoming is 1
    ///////////////////////////////////////////////////////////////////////////////////  vibration
    if(content == "1") {
      receivedValue = content.toInt(); 
      Serial.print("Received '1' with topic '");
      Serial.print(topic);
      Serial.println("'");
      digitalWrite(vibrationMotorPin, HIGH); // 开启震动
      delay(1000); // 震动1秒
      digitalWrite(vibrationMotorPin, LOW); // 关闭震动
      delay(1000); // 停止1秒
    //////////////////////////////////////////////////////////////////////////////////// if the incoming is 2  
    }else if(content == "2") {
      // 如果是，执行特定操作，比如存储这个值
      receivedValue = content.toInt(); // 将字符串"1"转换为整数1并存储
      // 打印接收到的消息，仅用于调试目的
      Serial.print("Received '2' with topic '");
      Serial.print(topic);
      Serial.println("'");
    //////////////////////////////////////////////////////////////////////////////////// if the incoming is 3
    }else if (content == "3") {
      // 如果是，执行特定操作，比如存储这个值
      receivedValue = content.toInt(); // 将字符串"1"转换为整数1并存储
      // 打印接收到的消息，仅用于调试目的
      Serial.print("Received '3' with topic '");
      Serial.print(topic);
      Serial.println("'");
    //////////////////////////////////////////////////////////////////////////////////// if the incoming is 4  
    }else if(content == "4") {
      // 如果是，执行特定操作，比如存储这个值
      receivedValue = content.toInt(); // 将字符串"1"转换为整数1并存储
      // 打印接收到的消息，仅用于调试目的
      Serial.print("Received '4' with topic '");
      Serial.print(topic);
      Serial.println("'");
    //////////////////////////////////////////////////////////////////////////////////// if the incoming is 5  
    }else if(content == "5") {
      // 如果是，执行特定操作，比如存储这个值
      receivedValue = content.toInt(); // 将字符串"1"转换为整数1并存储
      // 打印接收到的消息，仅用于调试目的
      Serial.print("Received '5' with topic '");
      Serial.print(topic);
      Serial.println("'");
      
    }
  }
}
void endRecording() {
  if (dataFile) {
    dataFile.close();
    Serial.println("录音结束，文件已保存。");
  }
}
