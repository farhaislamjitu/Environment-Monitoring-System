#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

/* ---------------- WiFi CREDENTIALS ---------------- */
const char* ssid = "arshu";          
const char* password = "12121212";   

/* ---------------- PIN DEFINITIONS ---------------- */
#define DHTPIN 27       
#define DHTTYPE DHT22   
#define MQ7_PIN 34
#define LED_PIN 19
#define BUZZER_PIN 18
#define SDA_PIN 21
#define SCL_PIN 22

/* ---------------- OBJECTS ---------------- */
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  
AsyncWebServer server(80);

/* ---------------- GLOBAL VARIABLES ---------------- */
float temperature = 0;
float humidity = 0;
int coValue = 0;
bool alertActive = false;
unsigned long uptimeSeconds = 0;

/* ---------------- CALIBRATION ---------------- */
const float TEMP_OFFSET = 6.6; 
const float HUM_OFFSET = 1.5;

/* ---------------- TIMING VARIABLES ---------------- */
unsigned long previousDHTMillis = 0;
unsigned long previousMQ7Millis = 0;
unsigned long previousLCDMillis = 0;
unsigned long previousAlertMillis = 0;
unsigned long previousUptimeMillis = 0;

const long dhtInterval = 10000;    
const long mq7Interval = 10000;    
const long lcdInterval = 10000;    
const long alertInterval = 500;   
const long uptimeInterval = 1000; 

/* HTML PAGE (আগের গর্জিয়াস ডিজাইনটি অক্ষুণ্ণ রাখা হয়েছে) */
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Environmental Monitor Dashboard</title>
    <style>
        * {margin: 0; padding: 0; box-sizing: border-box;}
        :root {
            --primary: #4299e1; --success: #48bb78; --warning: #ed8936;
            --danger: #f56565; --dark: #1a202c; --gray: #718096;
            --light: #edf2f7; --white: #ffffff;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh; padding: 20px; color: var(--dark);
        }
        .container {max-width: 1400px; margin: 0 auto;}
        header {
            background: var(--white); border-radius: 15px; padding: 30px;
            margin-bottom: 25px; box-shadow: 0 10px 25px rgba(0,0,0,0.15);
            text-align: center;
        }
        header h1 {font-size: 2.5rem; color: var(--dark); margin-bottom: 8px;}
        .subtitle {color: var(--gray); font-size: 1.1rem; margin-bottom: 20px;}
        .status-bar {
            display: flex; justify-content: space-between; align-items: center;
            padding-top: 20px; border-top: 2px solid var(--light);
            flex-wrap: wrap; gap: 15px;
        }
        .status-item {display: flex; align-items: center; gap: 8px; font-weight: 500;}
        .status-dot {
            width: 12px; height: 12px; border-radius: 50%;
            background: var(--gray); animation: pulse 2s infinite;
        }
        .status-dot.active {background: var(--success);}
        @keyframes pulse {0%, 100% {opacity: 1;} 50% {opacity: 0.5;}}
        .alert-banner {
            background: linear-gradient(90deg, var(--danger), #fc8181);
            color: var(--white); padding: 20px; border-radius: 12px;
            margin-bottom: 25px; text-align: center; font-weight: 700;
            font-size: 1.2rem; display: none; animation: alertPulse 1s infinite;
        }
        @keyframes alertPulse {0%, 100% {transform: scale(1);} 50% {transform: scale(1.02);}}
        .dashboard-grid {
            display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 25px; margin-bottom: 30px;
        }
        .card {
            background: var(--white); border-radius: 15px; padding: 25px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.15);
            transition: transform 0.3s ease; position: relative; overflow: hidden;
        }
        .card:hover {transform: translateY(-5px); box-shadow: 0 15px 35px rgba(0,0,0,0.2);}
        .card::before { content: ''; position: absolute; top: 0; left: 0; right: 0; height: 5px; }
        .temperature-card::before {background: linear-gradient(90deg, #fc8181, #f56565);}
        .humidity-card::before {background: linear-gradient(90deg, #63b3ed, #4299e1);}
        .co-card::before {background: linear-gradient(90deg, #f6ad55, #ed8936);}
        .status-card::before {background: linear-gradient(90deg, #9f7aea, #805ad5);}
        .card-icon {font-size: 3rem; margin-bottom: 15px;}
        .card-content h3 {
            color: var(--gray); font-size: 0.95rem; font-weight: 600;
            text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: 15px;
        }
        .reading {display: flex; align-items: baseline; gap: 8px; margin-bottom: 20px;}
        .reading .value {font-size: 3rem; font-weight: 700; color: var(--dark); line-height: 1;}
        .reading .unit {font-size: 1.5rem; color: var(--gray); font-weight: 500;}
        footer {
            background: var(--white); border-radius: 15px; padding: 20px;
            text-align: center; box-shadow: 0 4px 6px rgba(0,0,0,0.1); color: var(--gray);
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🌡️ Environmental Monitoring System</h1>
            <p class="subtitle">ESP32 Real-Time Dashboard (10s Refresh)</p>
            <div class="status-bar">
                <span class="status-item">
                    <span class="status-dot" id="statusDot"></span>
                    <span id="statusText">Connecting...</span>
                </span>
                <span class="uptime">Uptime: <span id="uptime">0h 0m 0s</span></span>
            </div>
        </header>

        <div class="alert-banner" id="alertBanner">
            ⚠️ SYSTEM ALERT: UNHEALTHY ENVIRONMENT DETECTED!
        </div>

        <div class="dashboard-grid">
            <div class="card temperature-card">
                <div class="card-icon">🌡️</div>
                <div class="card-content">
                    <h3>Temperature</h3>
                    <div class="reading">
                        <span class="value" id="temperature">--</span>
                        <span class="unit">°C</span>
                    </div>
                </div>
            </div>

            <div class="card humidity-card">
                <div class="card-icon">💧</div>
                <div class="card-content">
                    <h3>Humidity</h3>
                    <div class="reading">
                        <span class="value" id="humidity">--</span>
                        <span class="unit">%</span>
                    </div>
                </div>
            </div>

            <div class="card co-card">
                <div class="card-icon">☁️</div>
                <div class="card-content">
                    <h3>CO Level</h3>
                    <div class="reading">
                        <span class="value" id="coValue">--</span>
                        <span class="unit">ppm</span>
                    </div>
                </div>
            </div>

            <div class="card status-card">
                <div class="card-icon">⚙️</div>
                <div class="card-content">
                    <h3>Alert Status</h3>
                    <div class="reading">
                        <span class="value" id="alertStatus" style="font-size: 1.5rem;">NORMAL</span>
                    </div>
                </div>
            </div>
        </div>

        <footer>
            <p>ESP32 Monitoring | Update Interval: 10 Seconds | IP: <span id="ipDisplay">--</span></p>
        </footer>
    </div>

    <script>
        async function updateData() {
            try {
                const response = await fetch('/api/data');
                const data = await response.json();
                document.getElementById('temperature').textContent = data.temperature.toFixed(1);
                document.getElementById('humidity').textContent = data.humidity.toFixed(1);
                document.getElementById('coValue').textContent = data.co_value;
                document.getElementById('alertBanner').style.display = data.alert ? 'block' : 'none';
                document.getElementById('alertStatus').textContent = data.alert ? '🚨 ALERT' : '✅ NORMAL';
                document.getElementById('alertStatus').style.color = data.alert ? '#f56565' : '#48bb78';
                
                const hours = Math.floor(data.uptime / 3600);
                const minutes = Math.floor((data.uptime % 3600) / 60);
                const seconds = data.uptime % 60;
                document.getElementById('uptime').textContent = hours + "h " + minutes + "m " + seconds + "s";
                document.getElementById('statusDot').className = 'status-dot active';
                document.getElementById('statusText').textContent = 'Connected';
            } catch (error) {
                document.getElementById('statusDot').className = 'status-dot';
                document.getElementById('statusText').textContent = 'Disconnected';
            }
        }
        setInterval(updateData, 10000);
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.print("Rebooting...");
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n✅ Connected! IP: " + WiFi.localIP().toString());
  lcd.clear(); lcd.print("IP: "); lcd.print(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"co_value\":" + String(coValue) + ",";
    json += "\"alert\":" + String(alertActive ? "true" : "false") + ",";
    json += "\"uptime\":" + String(uptimeSeconds);
    json += "}";
    request->send(200, "application/json", json);
  });
  
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // DHT22 Read
  if (currentMillis - previousDHTMillis >= dhtInterval) {
    previousDHTMillis = currentMillis;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      temperature = t - TEMP_OFFSET;
      humidity = h - HUM_OFFSET;
    }
  }

  // MQ-7 Read
  if (currentMillis - previousMQ7Millis >= mq7Interval) {
    previousMQ7Millis = currentMillis;
    coValue = analogRead(MQ7_PIN);
  }

  // LCD Update
  if (currentMillis - previousLCDMillis >= lcdInterval) {
    previousLCDMillis = currentMillis;
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("T:" + String(temperature, 1) + " H:" + String(humidity, 0) + "%");
    lcd.setCursor(0, 1); 
    lcd.print("CO: " + String(coValue));
  }

  /* ---------------- NEW ALERT LOGIC ---------------- */
  if (currentMillis - previousAlertMillis >= alertInterval) {
    previousAlertMillis = currentMillis;
    
    // ১. CO লেভেল ১৫০০ পার হলে
    // ২. টেম্পারেচার ২৩ এর বেশি হলে
    // ৩. হিউমিডিটি ৪০ এর কম হলে
    if (coValue > 1500 || temperature > 23.0 || humidity < 40.0) {
      alertActive = true; 
      digitalWrite(LED_PIN, HIGH); 
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      alertActive = false; 
      digitalWrite(LED_PIN, LOW); 
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  // Uptime timer
  if (currentMillis - previousUptimeMillis >= uptimeInterval) {
    previousUptimeMillis = currentMillis; 
    uptimeSeconds++;
  }
}
