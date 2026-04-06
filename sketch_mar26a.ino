#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// encrypted data
#include "secrets.h"

//Declaring a variable containing DigiCert Global Root G2, standard for Azure encoding
const char* digicert_root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCG1IxCGhmZpNPvw1+He21C\n" \
"K4G02K9+x5/4J/o8yM7y5y9EwKxK5H7G26H/t19vR1K/g23lP+zTInz5n03o3O/O\n" \
"aE1mZ2+eP1yG42nC97iUo1D/E/xT4bU6T5L9R92q5U+9u+uH0Q1b9qL6v7n6X8t0\n" \
"5O7e9X6+v3t+v4Y8O9c/5w+vO/0u3m4+7x7T9s8+5X9y7m+0q1g9g1+7Q/7a3T9\n" \
"9P8V/E9X7X/5v6r6c7W+6A4+2A7W2x+z7B7s1+8P3+5v2m4q+9n2+Q/7b2W+8B\n" \
"8K/8d+q7x+9Q/z8+4y6n8+2K9/5c9s2/2u5/7e4+1Q+6r7QIDAQABo0IwQDAPBgNV\n" \
"HRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV5uNu\n" \
"5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAK/9a+9V/T9+0+6N+7k+8O+2\n" \
"W7A1P/3m6/9D9r7k/3M7T/8X5n+5p8n+8P3+5v2m4q+9n2+Q/7b2W+8B8K/8d+q7\n" \
"x+9Q/z8+4y6n8+2K9/5c9s2/2u5/7e4+1Q+6r7Q+8w+8e+8R+8a+8D/8b+8c+8\n" \
"-----END CERTIFICATE-----\n";

// Declaring a virtual 'client' with capability to encode SSL movement and communicate in MQTT
WiFiClientSecure espClient;
PubSubClient client(espClient);

const char* ssid = SECRET_WIFI_SSID;
const char* password = SECRET_WIFI_PASS;
const char* host = "Adrian-CyberLab-Hub.azure-devices.net";
const char* deviceId = "ESP32";
const char* sasToken = SECRET_AZURE_SAS_TOKEN;
const int buttonPin = 14;
const int ledPin = 2;

void callback(char* topic, byte* message, unsigned int message_len) {
  Serial.print(topic);
  String text = "";
  for(int i = 0; i < message_len; i++) {
    text += (char)message[i];
  }
  if (text == "ON") {
    digitalWrite(ledPin, HIGH);
  }
  else if (text == "OFF") {
    digitalWrite(ledPin, LOW);
  }
}

void setup_wifi() {

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi successfully");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  long esp32_date = 1700000000; // 2023
  while (now < esp32_date) {
      now = time(nullptr);
      Serial.print(".");
      Serial.print(ctime(&now));
      delay(500);   
  }
}


void reconnect() {

  String username = String(host) + "/" + deviceId + "/?api-version=2021-04-12";
  while (!client.connected()) {
    Serial.print("Trying to connect with Azure...");
    if (client.connect(deviceId, username.c_str() , sasToken)) {
      // topic fo cloud to device command
      String subscribeTopic = "devices/" + String(deviceId) + "/messages/devicebound/#";
      client.subscribe(subscribeTopic.c_str());
      Serial.println("Connection with Azure achieved Subscribed to C2D messages");
    }
    else {
      Serial.print("Error, code: "); 
      Serial.print(client.state());
      Serial.println("Connection failed. Trying again in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  // Configuring the virtual client
  espClient.setInsecure(); // Tells ESP32 to only trust servers signed by DigiCert
  client.setServer(host, 8883); // Function setting the target, our host and MQTT port which always equals 8883
  setup_wifi();
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) 
    reconnect();
  client.loop();
  int buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    Serial.println("Button pressed, building JSON...");
    // generating random sensor value for json file
    int currentSensorValue = random(10, 100);
    
    // Reserving memory for JSON
    StaticJsonDocument<200> doc;
    
    // FIlling out dictionary with needed data
    doc["device"] = "ESP32-Lab";
    doc["status"] = "ALARM_BUTTON_PRESSED";
    doc["sensor_value"] = currentSensorValue;
    
    // Initializng buffer of chars and serializing it with JSON
    char buffer[256];
    serializeJson(doc, buffer);

    // Building targetted address (Topic), which Azure is monitoring
    // Required formatting: devices/{Twoj_Device_ID}/messages/events/
    String topic = "devices/";
    topic += deviceId;
    topic += "/messages/events/";

    // Sending the message. Publish() returns true when successfull
    if (client.publish(topic.c_str(), buffer)) {
      Serial.println("Message successfully sent to Azure");
    } else {
      Serial.println("Error sending message");
    }
    delay(3000);
  }
}
