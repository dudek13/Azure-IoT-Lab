# Project: Azure Iot Secure Edge Controller (ESP32)

Summary: Monitoring and Edge Device controlling system integrated with **Microsoft Azure**.
Project demonstrates safe, bidirectional MQTT communication and automatization of business procedures in cloud.

---

## System's architecture

Project is based on two data flow directions:
1. **D2C (Device-to-Cloud):** ESP32 sends alerts in a form of JSON (device status and sensor value) after event detection (pressed button).
2. **C2D (Cloud-to-Device):** Azure cloud sends control commands (ON/OFF) directly to ESP32, using *MQTT Callback*.

---

## Key functionalities
* **Safe communication:** Used MQTT protocol through 8883 port with TLS/SSL certificates.
* **Authentication:** Protected access to Azure IoT Hub using SAS (Shared Access Signature) token.
* **Sensitive Data Management:** Full separation of sensitive data (Wifi ssid and password, Azure keys) from source code using 'secrets.h' and '.gitignore'.
* **Cloud automation:** Integration with **Azure Logic Apps**, which automatically sends e-mail notification after receiving telemetery from device.
* **Time synchronization:** Used NTP protocol for correct certifactions and tokens validation.

---

## Hardware and Technology Stack

| Category | Tools / Technologies |
| :--- | :--- |
| **Hardware** | ESP32, Button, LED |
| **Cloud Platform** | Microsoft Azure (IoT Hub, Logic Apps) |
| **Protocols** | MQTT, HTTPS, NTP |
| **Language/Libraries** | C++, ArduinoJson, PubSubClient, WiFiClientSecure |
| **Tools** | Git, Azure IoT Explorer |

---

## How to run the project ?

1. **Clone the repository**: 'git clone https://github.com/dudek13/Azure-IoT-Lab'
2. **Secrets configuration:** 
	* In main folder you'll find: 'secrets_example.h'.
	* Change it's name to: 'secrets.h'
	* Fill in the parameters: 'SECRETS_WIFI_SSID', 'SECRETS_WIFI_PASS' 									and 'SECRET_AZURE_TOKEN'.
3. **Compiling:** Open '.ino' file in Arduino IDE, install the required libraries and upload the code into ESP32.
4. **Testing:** Use *Azure IoT Explorer* or Azure portal to send the commands 'ON'/'OFF' and observe led reaction.

---

## Safety
'secrets.h' file was excluded from the repository using '.gitignore' in order to prevent any important data leak.
