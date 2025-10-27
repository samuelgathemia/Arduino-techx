# Arduino-techx
# 🌡️ ESP32 Sensor Dashboard

[![Platform](https://img.shields.io/badge/Platform-ESP32-blue?logo=espressif)](https://www.espressif.com/)
[![Language](https://img.shields.io/badge/Language-C++-brightgreen?logo=c%2B%2B)](https://isocpp.org/)
[![Library](https://img.shields.io/badge/Library-DHT11-orange)](https://github.com/adafruit/DHT-sensor-library)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Status](https://img.shields.io/badge/Status-Active-success.svg)]()

---

This project creates a **Wi-Fi-enabled environmental monitoring dashboard** using an **ESP32** and a **DHT11 temperature & humidity sensor**.  
It hosts a local web server that displays **real-time sensor readings** and allows **LED control** directly from your browser.

---

## 🚀 Features

- 📶 Creates a **Wi-Fi Access Point** (`ESP32_Dashboard`)
- 🌡️ Real-time **temperature** and **humidity** readings from DHT11
- 💡 Control the onboard **LED** via a toggle button
- 🔄 **Auto-refreshes** every 2 seconds using JavaScript (Fetch API)
- 🎨 Clean, mobile-friendly **HTML + CSS** dashboard UI
- ⚙️ Runs **entirely offline** — no internet required

---

## 🧩 Hardware Requirements

| Component | Description |
|------------|--------------|
| ESP32 | Wi-Fi capable microcontroller |
| DHT11 Sensor | For temperature & humidity sensing |
| Jumper Wires | For basic connections |
| Breadboard | Optional, for prototyping |

---

## ⚡ Circuit Connections

| DHT11 Pin | ESP32 Pin |
|------------|-----------|
| VCC | 3.3V |
| GND | GND |
| DATA | GPIO 4 |

The onboard **LED (GPIO 2)** is used for LED control.

---

## 🧠 How It Works

1. The ESP32 starts in **Access Point mode** and hosts a local web server.  
2. The DHT11 sensor reads **temperature and humidity** every 2 seconds.  
3. The dashboard fetches data via **AJAX (Fetch API)** and updates the UI dynamically.  
4. Clicking the **Toggle LED** button sends a request to the ESP32 to flip the LED state.

---

## 🧾 Web Dashboard Preview

📱 ESP32 Sensor Dashboard
├── Temperature: 25 °C
├── Humidity: 60 %
└── LED Control: [Toggle LED]


The readings refresh automatically every 2 seconds.

---

## 🔧 Setup and Usage

### 1️⃣ Install Dependencies
In the Arduino IDE, make sure the following libraries are installed:
- `WiFi.h` (built-in with ESP32 core)
- `WebServer.h` (built-in with ESP32 core)
- `DHT.h` (install via Library Manager → “DHT sensor library by Adafruit”)

### 2️⃣ Upload the Code
1. Connect your ESP32 board to your computer.
2. Copy this code into the Arduino IDE.
3. Select **Tools → Board → ESP32 Dev Module**.
4. Upload the sketch to your ESP32.

### 3️⃣ Connect and Access Dashboard
1. Open **Serial Monitor** at `115200 baud` and note the IP address, e.g.:
AP IP: 192.168.4.1

2. Connect to the Wi-Fi network:
SSID: ESP32_Dashboard
Password: 12345678

3. Open your web browser and go to:
http://192.168.4.1/


🎉 You’ll now see your live dashboard showing temperature, humidity, and LED control!

---

## 🧰 Code Overview

| Function | Description |
|-----------|--------------|
| `handleRoot()` | Serves the main HTML dashboard |
| `handleSensor()` | Returns JSON with temperature & humidity |
| `handleToggleLED()` | Toggles the onboard LED and returns its new state |
| `loop()` | Handles client requests and updates sensor readings periodically |

---

## 🛡️ Notes

- DHT11 sensors are slow; readings are updated every **2 seconds**.
- The ESP32 runs in **Access Point mode**, no internet required.
- You can modify it to run in **Station mode** to connect to existing Wi-Fi networks.

---

## 🧑‍💻 Author

**samuel gathemia**  
Electrical & Electronics Engineer | Embedded Systems & IoT Developer  
📍 South Eastern Kenya University (SEKU)  
💡 Passionate about automation, IoT, and real-time monitoring systems.

---

## 📜 License

This project is open-source and available under the **MIT License**.  
Feel free to use, modify, and share.

---

## ⭐ Support

If you like this project, please **give it a ⭐ on GitHub** — it helps others discover it!

---
