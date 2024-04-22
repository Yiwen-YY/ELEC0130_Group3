const int buttonPin = 0;     // 按钮连接到数字引脚2
int buttonState = 0;         // 变量用于读取按钮状态

void setup() {
  Serial.begin(9600);          // 初始化串行通讯
  pinMode(buttonPin, INPUT_PULLUP);  // 将按钮引脚设置为输入，并启用内置上拉电阻
}

void loop() {
  buttonState = digitalRead(buttonPin); // 读取按钮状态
  if (buttonState == LOW) {             // 检查按钮是否被按下
    Serial.println("Button Pressed");   // 如果是，打印消息
    while(digitalRead(buttonPin) == LOW); // 简单的防抖动处理，等待按钮释放
  }
}
