#include <ArduinoBLE.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000

#include "arduino_secrets.h"

const char* targetPeripheralAddress = "3c:61:05:16:43:16";  // Replace with the address of your specific peripheral
bool deviceDiscovered = false; // Flag to indicate if the target device has been discovered

// Define the known locations of the central devices
float central1X = 120;  // Replace with the actual X coordinate of central1
float central1Y = 20;   // Replace with the actual Y coordinate of central1

float central2X = 40;  // Replace with the actual X coordinate of central2
float central2Y = 90;  // Replace with the actual Y coordinate of central2

float estimatedX = 0.0;
float estimatedY = 0.0;

struct Point {
  float x;
  float y;

  // Constructors
  Point() : x(0), y(0) {}  // Default constructor with coordinates (0, 0)
  Point(float xVal, float yVal) : x(xVal), y(yVal) {} // Constructor to initialize x and y coordinates
};

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = "Andria";
const char pass[]        = "12345678";
const char broker[]      = "a2bu9nf5celvb1-ats.iot.eu-west-2.amazonaws.com";
const char* certificate  = R"(
-----BEGIN CERTIFICATE-----
MIICiTCCAXGgAwIBAgIVAPW2tg5xvgcH81kL7tMW4AzTMCk8MA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDAzMTAwMjU5
NDdaFw00OTEyMzEyMzU5NTlaMBgxFjAUBgNVBAMTDU15TUtSV2lGaTEwMTAwWTAT
BgcqhkjOPQIBBggqhkjOPQMBBwNCAAT3ysMVBlLV6EUMCcyEbSYWJ5dFCWgAMM3j
I/bp4Oh90Rmqx9PDCVghmYyKLn840xwOAqV2rM7e7IahS/gXkYCjo2AwXjAfBgNV
HSMEGDAWgBR+xRl7tTHacZe7ZqsewWL/aJ7adzAdBgNVHQ4EFgQUSK8pbaxuY0uE
2pR66lYsK6Mp4IwwDAYDVR0TAQH/BAIwADAOBgNVHQ8BAf8EBAMCB4AwDQYJKoZI
hvcNAQELBQADggEBAJyGuEKLCzIerhVa9C/hZGqAaoEavrLqT/xqiAwUHkuMQzt0
V5Matt6S0fkD7S263uWTpNPPirKuqErt6DN4ZZSJpjHAzUIxRHCVe9g8t2TZyn0I
3/k/6rCFVEzbwEtqhHYcwPXqFh0bHnN+uzneJIDOiF7LRtmlp96NaDf44WbKmKdR
BriiAcsacVfx65yo4mzAcK5MM29wIhEBGqNGuTaqsHW5ehsFMD2KUOfMfB5QTDys
oKUpeSExD4LoZWfmjYp0VmM4xvrwvlHVfFz6y6kd0k/1hy+m2liuqqqQl5Gzb2yG
ovQHXLC7xshPT+oru3WRhwUgqKqkk1VUkpCn3h0=
-----END CERTIFICATE-----
)";

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;

void setup() {
  //Serial.begin(9600);
  //while (!Serial);

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  Serial.print("Bluetooth address: ");
  Serial.println(BLE.address());

  // start scanning for peripheral
  BLE.scan();
  Serial.println("Bluetooth® Low Energy Central scan");

  
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
  mqttClient.onMessage(onMessageReceived);
}

void loop() {
  if (!deviceDiscovered) {
    BLEDevice peripheral = BLE.available();

    if (peripheral) {
      Serial.println("Peripheral discovered");

      if (peripheral.address() == targetPeripheralAddress) {
        Serial.println("Discovered the specific peripheral");
        Serial.println("----------------------------------");
        Serial.print("Address: ");
        Serial.println(peripheral.address());

        if (peripheral.hasLocalName()) {
          Serial.print("Local Name: ");
          Serial.println(peripheral.localName());
        }

        if (peripheral.hasAdvertisedServiceUuid()) {
          Serial.print("Service UUIDs: ");
          for (int i = 0; i < peripheral.advertisedServiceUuidCount(); i++) {
            Serial.print(peripheral.advertisedServiceUuid(i));
            Serial.print(" ");
          }
          Serial.println();
        }

        int rssi = peripheral.rssi();
        Serial.print("RSSI: ");
        Serial.println(rssi);

        estimateLocation(rssi, peripheral.address().c_str());
        deviceDiscovered = true;
      }
    }
  } else {
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }

    if (!mqttClient.connected()) {
      connectMQTT();
    }

    mqttClient.poll();

    if (millis() - lastMillis > 1000) {
      lastMillis = millis();
      publishMessage();
    }
  }
}

void estimateLocation(int rssi, const char* peripheralAddress) {
  Serial.println("Estimating location...");
  // Define the RSSI-distance model (you may need to adjust this based on your environment)
  float distance = estimateDistance(rssi);

  // Determine which central device provided the RSSI measurement
  Point centralLocation;
  if (strcmp(peripheralAddress, "3c:61:05:16:43:16") == 0) {
    centralLocation = Point(central1X, central1Y);
  } else if (strcmp(peripheralAddress, "9c:9c:1f:c1:61:aa") == 0) {
    centralLocation = Point(central2X, central2Y);
  } else {
    // Unknown central device
    return;
  }

 // Triangulation
  Point estimatedPeripheralLocation = triangulate(centralLocation, distance, rssi);

  // Update global variables with the estimated location
  estimatedX = estimatedPeripheralLocation.x;
  estimatedY = estimatedPeripheralLocation.y;

  // Print the estimated location
  Serial.print("Estimated Location: (");
  Serial.print(estimatedX);
  Serial.print(", ");
  Serial.print(estimatedY);
  Serial.println(")");
}

float estimateDistance(int rssi) {
  // Replace this with your distance estimation model
  // Example: Free Space Path Loss (FSPL) model
  // FSPL(dB) = 20 * log10(d) + 20 * log10(f) + 20 * log10(4π/c)
  // where d is the distance, f is the frequency, and c is the speed of light

  // For simplicity, you can use a linear model or other empirical models
  // This example assumes a linear model with a reference distance and reference RSSI
  float referenceDistance = 1.0;  // Reference distance in meters
  int referenceRSSI = -60;        // Reference RSSI at the reference distance

  // Calculate estimated distance based on the linear model
  float estimatedDistance = referenceDistance * pow(10, (referenceRSSI - rssi) / (10 * 2.0));

  return estimatedDistance;
}

// Triangulation function
Point triangulate(Point centralLocation, float distance, int rssi) {
  // Simple triangulation: Move distance units away from the central location
  Point estimatedPeripheralLocation;
  estimatedPeripheralLocation.x = centralLocation.x + distance;

  // Adjust the Y-coordinate based on RSSI or other parameters
  // You may need to experiment and calibrate this based on your environment
  estimatedPeripheralLocation.y = centralLocation.y + map(rssi, -100, -50, -5, 5);

  return estimatedPeripheralLocation;
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

void publishMessage() {
  Serial.println("Publishing message with estimated location");

  mqttClient.beginMessage("arduino/outgoing4");
  mqttClient.print("{\"x\": ");
  mqttClient.print(estimatedX);
  mqttClient.print(", \"y\": ");
  mqttClient.print(estimatedY);
  //mqttClient.print(", \"timestamp\": ");
  mqttClient.print(millis());
  mqttClient.println("}");
  mqttClient.endMessage();
}

void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();
}
