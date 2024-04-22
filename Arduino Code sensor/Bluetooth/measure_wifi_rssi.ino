#include <WiFiNINA.h>

char ssid[] = "Andria";
char pass[] = "12345678";

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Connect to WiFi
  if (WiFi.begin(ssid, pass) == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
  } else {
    Serial.println("Connection failed!");
    while (true); // Hang in case of connection failure
  }

  // Enable Bluetooth
  Serial1.begin(9600); // Serial1 is used for Bluetooth communication
  Serial.println("Bluetooth is ready!");
}

void loop() {
  // Measure RSSI
  int rssi = WiFi.RSSI();

  // Print RSSI value
  Serial.print("RSSI: ");
  Serial.println(rssi);

  // Your main code here
  delay(1000); // Adjust the delay according to your needs
}
