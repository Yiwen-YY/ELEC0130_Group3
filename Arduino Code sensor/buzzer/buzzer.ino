const int buzzerPin = 3; // 将蜂鸣器连接到数字引脚9

void setup() {
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  tone(buzzerPin, 500); // 在引脚上生成1000Hz的音调
  delay(1000); // 持续1秒
  noTone(buzzerPin); // 停止发声
  delay(1000); // 等待1秒
}
