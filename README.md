# 💡 SmartLights — Dual Mode ESP32 Wi-Fi Lighting Controller

[![Platform](https://img.shields.io/badge/Platform-ESP32-blue?logo=espressif)](https://www.espressif.com/)
[![Language](https://img.shields.io/badge/Language-C++-brightgreen?logo=c%2B%2B)](https://isocpp.org/)
[![Network](https://img.shields.io/badge/Mode-SoftAP%20+%20STA-yellow)]()
[![UI](https://img.shields.io/badge/UI-Web%20Dashboard-orange)]()
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)](LICENSE)

SmartLights is a dual-mode ESP32 web-based lighting control system.  
It combines **SoftAP + Station (STA)** modes, allowing users to either connect directly to the ESP32 access point or control it over a local Wi-Fi network.

---

## 🚀 Features

- **Dual Mode Operation:** SoftAP + STA (access point + router connection)
- **Interactive Web UI:** Two pages — Dashboard and Settings
- **Dynamic Frontend:** Animated toggles, live status updates, responsive layout
- **Wi-Fi Configuration:** Scan, save, and auto-connect to known networks
- **mDNS Support:** Access device via `<device_name>.local`
- **Persistent Storage:** Saves credentials and device name using Preferences
- **Silent Operation:** No serial prints; designed for standalone use

---

## 🧱 Hardware Setup

| Component | Pin |
|------------|-----|
| Kitchen Light  | GPIO 2 |
| Bedroom Light  | GPIO 4 |
| Sitting Room   | GPIO 16 |
| Security Light | GPIO 17 |
| Balcony Light  | GPIO 18 |

---

## 🌐 Web Interface Overview

### 🖥 Dashboard (`/`)
- Displays toggle switches for each light  
- Animated background that changes with light activity  
- Shows access mode (local or AP)  
- Periodic auto-refresh every 3 seconds  

### ⚙️ Settings (`/settings`)
- Scans for nearby Wi-Fi networks  
- Lets you set SSID, password, and custom device name  
- Connects to Wi-Fi and starts mDNS service  
- Persists credentials via `Preferences`

---

## 📦 Code Overview

### Core Libraries Used:
```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
Light Configuration
cpp
Copy code
const int PIN_KITCHEN  = 2;
const int PIN_BEDROOM  = 4;
const int PIN_SITTING  = 16;
const int PIN_SECURITY = 17;
const int PIN_BALCONY  = 18;
Light Control Structure
cpp
Copy code
struct Light {
  const char* name;
  int pin;
  bool state;
};
Web Endpoints
Endpoint	Method	Purpose
/	GET	Loads the Dashboard
/settings	GET	Loads Wi-Fi configuration page
/status	GET	Returns light states in JSON
/toggle?id=<name>	GET	Toggles specific light
/scan	GET	Scans for available Wi-Fi networks
/savewifi	POST	Saves Wi-Fi and device name

🧠 Backend Logic Summary
Initializes all GPIO pins as outputs.

Loads stored credentials from Preferences.

Starts ESP32 as an Access Point (ESP32-AP-Lights).

If credentials exist, tries connecting to the saved router (STA mode).

Launches an internal web server with all routes.

Registers mDNS using the configured device name.

Responds to HTTP requests to control or configure lights.

🌈 Frontend Highlights
Dashboard UI
Modern dark design with animated toggles.

Dynamic background gradients reflect active lights.

Automatic refresh for real-time updates.

Settings UI
Sleek interface for scanning networks.

Save and connect options with feedback messages.

Displays mDNS access URL upon success.

🧰 Example JSON Response
When visiting /status, you’ll receive:

json
Copy code
{
  "kitchen": 1,
  "bedroom": 0,
  "sitting": 1,
  "security": 0,
  "balcony": 0
}
⚙️ Setup Instructions
Open your Arduino IDE or PlatformIO.

Install libraries: WiFi, WebServer, ESPmDNS, Preferences.

Upload the sketch to your ESP32.

Connect to ESP32-AP-Lights using password esp32lights.

Visit http://192.168.4.1/ to access the web dashboard.

Go to Settings to connect your ESP32 to your Wi-Fi network.

Access your device later via http://<device_name>.local.

📁 Project Directory Layout
kotlin
Copy code
SmartLights/
├── SmartLights.ino
├── README.md
├── LICENSE
└── data/
    ├── index.html
    ├── settings.html
🧾 Notes
Operates fully offline in AP mode.

Designed for smart home automation and IoT experiments.

No serial output; runs quietly for stable field deployment.

🧑‍💻 Author
Developed by Samuel Gathemia
Electrical and Electronics Engineering
South Eastern Kenya University (SEKU)
