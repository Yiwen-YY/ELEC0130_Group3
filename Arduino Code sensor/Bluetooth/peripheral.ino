#include <ArduinoBLE.h>

BLEService myService("fff0");
BLEIntCharacteristic myCharacteristic("fff1", BLERead | BLEBroadcast);

// Advertising parameters should have a global scope. Do NOT define them in 'setup' or in 'loop'
const uint8_t completeRawAdvertisingData[] = {0x02,0x01,0x06,0x09,0xff,0x01,0x01,0x00,0x01,0x02,0x03,0x04,0x05};   

void setup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    while (1);
  }

  // Print the Bluetooth address of the peripheral
  Serial.print("Bluetooth address: ");
  Serial.println(BLE.address());

  myService.addCharacteristic(myCharacteristic);
  BLE.addService(myService);

  // Build advertising data packet
  BLEAdvertisingData advData;
  // If a packet has raw data, then other parameters of the packet will be ignored
  advData.setRawData(completeRawAdvertisingData, sizeof(completeRawAdvertisingData));  
  // Copy set parameters in the actual advertising packet
  BLE.setAdvertisingData(advData);

  // Build scan response data packet
  BLEAdvertisingData scanData;
  scanData.setLocalName("Mobile Device");
  // Copy set parameters in the actual scan response packet
  BLE.setScanResponseData(scanData);
  
  BLE.advertise();

  Serial.println("Advertising...");
}

void loop() {
  BLE.poll();
}
