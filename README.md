# Project: Azure IoT SIEM & Edge Security Controller (ESP32 + Splunk)

**Summary:** An advanced Edge Device monitoring and security logging system integrated with **Microsoft Azure** and a local **Splunk SIEM** environment. 
This project demonstrates secure IoT communication, cloud automation, and Infrastructure as Code (IaC) using Docker containerization to build a fully functional Security Operations Center (SOC) data pipeline.

---

## System Architecture

The project consists of a complete data pipeline routing telemetry and security events from the physical edge to a local SIEM:
1. **Device-to-Cloud (D2C):** ESP32 detects physical events (e.g., button press/alarm) and sends a JSON payload to Azure via HTTP POST / MQTT.
2. **Cloud Orchestration:** **Azure Logic Apps** catches the payload and forwards it to a dynamically generated Ngrok tunnel.
3. **Tunneling & Containerization:** The Ngrok container securely routes the public traffic into a private Docker network.
4. **SIEM Ingestion:** **Splunk Enterprise** receives the data via HTTP Event Collector (HEC) on port 8088 and indexes it for real-time threat monitoring and dashboarding.
5. **Cloud-to-Device (C2D) *(Optional/Legacy)*:** Azure can send control commands (ON/OFF) directly back to the ESP32.

---

## Key Functionalities
* **Containerized SIEM Environment:** Splunk Enterprise and Ngrok are deployed automatically using `docker-compose`, eliminating manual installation and ensuring environmental consistency.
* **Cloud Automation:** Integration with **Azure Logic Apps** to route traffic and act as a middleware bridge between the IoT device and the local server.
* **Sensitive Data Management (DevSecOps):** Strict separation of credentials. WiFi/Azure keys are secured via `secrets.h` on the device, while Splunk/Ngrok tokens are secured via `.env` files in Docker. Git ignores both.
* **Hybrid Security & SSL Offloading:** Public data transit (Azure to Ngrok endpoint) is fully secured via HTTPS/TLS. For local container communication, SSL offloading is applied, routing plain HTTP from Ngrok directly to Splunk. Device-to-Cloud communication utilizes MQTT over TLS (port 8883) with SAS token authentication.

---

## Hardware and Technology Stack

| Category | Tools / Technologies |
| :--- | :--- |
| **Hardware** | ESP32, Button, LED |
| **Cloud Platform** | Microsoft Azure (Logic Apps, IoT Hub) |
| **Local Server** | Docker, Docker Compose |
| **Monitoring / SIEM** | Splunk Enterprise, Ngrok |
| **Protocols** | HTTP(S) POST, MQTT, NTP |
| **Language/Libraries** | C++, ArduinoJson, Bash |

---

## How to run the project?

### Phase 1: Local SIEM Setup (Docker)
1. **Clone the repository**: `git clone https://github.com/dudek13/Azure-IoT-Lab`
2. **Environment Configuration:** * Locate the `.env.example` file in the main directory.
   * Rename it or copy it to: `.env`
   * Fill in your secure parameters: `SPLUNK_ADMIN_PASS` (min. 8 characters) and `NGROK_TOKEN`.
3. 3. **Start the Infrastructure:** We use a custom Bash script to automate the deployment and Ngrok routing. Run:
   ```bash
   chmod +x start_env.sh
   ./start_env.sh
4. **Splunk Configuration:**
   * Go to `http://localhost:8000` and log in.
   * Navigate to **Settings -> Data Inputs -> HTTP Event Collector**.
   * Go to **Global Settings**, ensure Tokens are *Enabled* and **Disable SSL** (critical for local Docker traffic).
   * Create a new Token, assign it to the `main` index, and copy it.

### Phase 2: Hardware Setup (ESP32)
1. **Secrets configuration:** * Locate `secrets_example.h` in the ESP32 code folder.
   * Rename it to `secrets.h`.
   * Fill in your `WIFI_SSID`, `WIFI_PASS`, and cloud endpoint/tokens.
2. **Flashing:** Open the `.ino` file in Arduino IDE, install required libraries, and upload the code to your ESP32.

### Phase 3: Cloud Routing
1. The `start_env.sh` script will automatically fetch and display your new public Ngrok URL in the terminal. Copy this URL.
2. Update your **Azure Logic App** HTTP action with the new Ngrok URL (append `/services/collector/event` at the end) and insert your new Splunk HEC Token in the headers.

---

## Security & Privacy (Zero Trust)
To prevent critical data leaks, both `.env` and `secrets.h` files are permanently excluded from the repository using the `.gitignore` file. Never commit raw passwords or tokens.

---

## Troubleshooting (Issues & Solutions)

* **Issue:** Splunk container crashes immediately after startup (Status: Exited 1).
  * **Solution:** Splunk requires explicit acceptance of the General Terms. Added `SPLUNK_GENERAL_TERMS=--accept-sgt-current-at-splunk-com` to `docker-compose.yml`. Ensure the password is at least 8 characters long.
* **Issue:** Ngrok tunnel times out (ERR_NGROK_8012) and fails to forward traffic to Splunk.
  * **Solution:** Splunk HEC defaults to encrypted HTTPS. Because Ngrok handles the public HTTPS and forwards plain HTTP internally, you must uncheck **"Enable SSL"** in Splunk's HEC Global Settings.
* **Issue:** Git push rejected (Divergent Branches / Fetch First).
  * **Solution:** The remote repository contained files not present locally (e.g., initialized README). Ran `git config pull.rebase false`, followed by `git pull origin main` to merge, and then successfully pushed using `git push origin main`.
