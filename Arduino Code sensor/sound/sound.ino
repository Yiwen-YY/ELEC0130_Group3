const int soundSensorPin = A0; // 声音传感器连接到Arduino的A0引脚
int sensorValue = 0; // 用于存储传感器值的变量

void setup() {
  Serial.begin(9600); // 初始化串行通信
}

void loop() {
  sensorValue = analogRead(soundSensorPin); // 读取声音传感器的模拟值
  Serial.println(sensorValue); // 将读取的值打印到串行监视器
  delay(100); // 短暂延时以便于观察
}
