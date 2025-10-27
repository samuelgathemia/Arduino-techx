#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#define DHTPIN 4     // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT11   // DHT 11

#define ledPin 2 // On-board LED pin

DHT dht(DHTPIN, DHTTYPE);
float temperature = 0.0;
float humidity = 0.0;

// Read sensor at regular intervals (DHT11 needs ~2s between reads)
const unsigned long SENSOR_INTERVAL_MS = 2000;
unsigned long lastSensorReadMs = 0;

// Timing for LED blinking
const unsigned long LED_INTERVAL_MS = 1000;
unsigned long lastLedToggleMs = 0;
bool ledState = LOW;

const char *ssid = "ESP32_Dashboard";
const char *password = "12345678";

// Create a web server on port 80
WebServer server(80);

// HTML + CSS + JS for dashboard
String dashboardPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Sensor Dashboard</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background: #f4f4f4; margin: 0; }
    h1 { background: #0073e6; color: white; padding: 20px; margin: 0; }
    .card { background: white; border-radius: 10px; padding: 20px; margin: 20px auto; width: 300px; 
            box-shadow: 0 4px 8px rgba(0,0,0,0.2); }
    .value { font-size: 2em; color: #0073e6; }
    .label { font-size: 1em; color: #333; }
    button { background: #0073e6; color: white; border: none; padding: 10px 20px; margin-top: 15px;
             font-size: 1em; border-radius: 5px; cursor: pointer; }
    button:hover { background: #005bb5; }
  </style>
  <script>
    function fetchData(){
      fetch('/sensor').then(r => r.json()).then(data => {
        document.getElementById('temp').innerHTML = data.temperature + " °C";
        document.getElementById('hum').innerHTML = data.humidity + " %";
      });
    }
    function toggleLED(){
      fetch('/toggleLED').then(r => r.text()).then(state => {
        document.getElementById('ledStatus').innerHTML = "LED is " + state;
      });
    }
    setInterval(fetchData, 2000);
  </script>
</head>
<body>
  <h1>🌡️ ESP32 Sensor Dashboard</h1>
  <div class="card">
    <div class="label">Temperature</div>
    <div id="temp" class="value">-- °C</div>
  </div>
  <div class="card">
    <div class="label">Humidity</div>
    <div id="hum" class="value">-- %</div>
  </div>
  <div class="card">
    <div class="label">LED Control</div>
    <button onclick="toggleLED()">Toggle LED</button>
    <div id="ledStatus" style="margin-top:10px;">LED is OFF</div>
  </div>
</body>
</html>
)rawliteral";


    void handleRoot() {
      server.send(200, "text/html", dashboardPage);
    }

    void handleSensor() {
      String json = "{ \"temperature\": " + String(temperature) +
                    ", \"humidity\": " + String(humidity) + " }";
      server.send(200, "application/json", json);
    }

    void handleToggleLED() {
      ledState = !ledState;                  // flip LED state
      digitalWrite(ledPin, ledState);
      String stateStr = ledState ? "ON" : "OFF";
      server.send(200, "text/plain", stateStr);
    }

    void setup() {
      Serial.begin(115200);
      dht.begin();
      pinMode(ledPin, OUTPUT);

      // Wait for sensor to stabilize (DHT11 needs some time after power-up)
      delay(2000);

      WiFi.softAP(ssid, password);
      Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

      server.on("/", handleRoot);
      server.on("/sensor", handleSensor);
      server.on("/toggleLED", handleToggleLED);

      server.begin();
      Serial.println("Server started");
    }

    void loop() {
      server.handleClient();
      unsigned long now = millis();

      // Sensor update
      if (now - lastSensorReadMs >= SENSOR_INTERVAL_MS) {
        lastSensorReadMs = now;
        float newHum = dht.readHumidity();
        float newTemp = dht.readTemperature();
        if (isnan(newHum) || isnan(newTemp)) {
          Serial.println("Failed to read from DHT sensor! Keeping previous values.");
          // do nothing (keep old values)
        }
        else {
          humidity = newHum;
          temperature = newTemp;
          Serial.print("Temperature: "); Serial.print(temperature);
          Serial.print(" °C, Humidity: "); Serial.print(humidity);
          Serial.println(" %");
        }
      }
    }
