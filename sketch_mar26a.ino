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
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n" \
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n" \
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n" \
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n" \
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n" \
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n" \
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n" \
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n" \
"1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n" \
"NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n" \
"Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n" \
"8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n" \
"pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n" \
"MrY=\n" \
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
  // Configuring the virtual client with real security
  espClient.setCACert(digicert_root_ca); // Tells ESP32 to only trust servers signed by DigiCert
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
