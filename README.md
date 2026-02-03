# Environment Monitoring System (ESP32)

A cost-effective, IoT-enabled indoor environment monitor that measures temperature, humidity, and Carbon Monoxide (CO). Built around an ESP32, it provides a local I2C LCD display and a web dashboard (served from the device) for remote monitoring. The system raises audible and visual alerts (buzzer + LED) when safety thresholds are exceeded.

---

## Table of Contents
- [Features](#features)
- [Hardware](#hardware)
- [Wiring / Pin Mapping](#wiring--pin-mapping)
- [Software Requirements](#software-requirements)
- [Setup & Flashing](#setup--flashing)
- [Usage](#usage)
- [Calibration](#calibration)
- [Alert Logic ("Safety Brain")](#alert-logic-safety-brain)
- [Troubleshooting](#troubleshooting)
- [Results & Observations](#results--observations)
- [Future Improvements](#future-improvements)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

---

## Features
- Real-time monitoring of:
  - Temperature (DHT22)
  - Humidity (DHT22)
  - CO gas level (MQ-7 analog sensor)
- Local display using a 16x2 I2C LCD
- Web dashboard (served by ESP32 using Async web server)
- AJAX polling (default every 10s) for smooth live updates
- Audible (buzzer) and visual (LED) alerts
- Software calibration offsets for temperature and humidity

---

## Hardware
- ESP32 development board (dual-core, built-in Wi‑Fi)
- DHT22 temperature & humidity sensor
- MQ-7 Carbon Monoxide (CO) sensor (analog output)
- I2C 16x2 LCD (SDA / SCL)
- Active buzzer
- LED (with current‑limiting resistor)
- Breadboard, jumper wires, 5V/3.3V power as required

Component summary:
- ESP32: main controller, Wi‑Fi host
- DHT22: digital temp & humidity sensor (GPIO 27)
- MQ-7: analog CO sensor (GPIO 34 / ADC)
- I2C LCD: SDA GPIO 21, SCL GPIO 22
- Buzzer: GPIO 18
- LED: GPIO 19

---

## Wiring / Pin Mapping
Connect as follows (ESP32 GPIO references):

- DHT22 data -> GPIO 27 (digital input)
- MQ-7 analog output -> GPIO 34 (ADC)
- Buzzer (active) -> GPIO 18 (digital output)
- LED -> GPIO 19 (digital output) with resistor
- LCD SDA -> GPIO 21 (I2C SDA)
- LCD SCL -> GPIO 22 (I2C SCL)
- Common ground between ESP32 and sensors

Note: MQ-7 requires correct heater drive sequence in some designs (check the sensor datasheet). MQ sensors typically expect 5V for heater; the analog output must be within ESP32 ADC range and properly referenced.

---

## Software Requirements
- Arduino IDE (or PlatformIO)
- ESP32 board support installed
- Required Arduino libraries:
  - ESPAsyncWebServer
  - AsyncTCP (dependency for ESPAsyncWebServer on ESP32)
  - DHT sensor library (e.g., "DHT sensor library" by Adafruit) and Adafruit_Sensor adapter if used
  - LiquidCrystal_I2C (or whichever I2C LCD library your code expects)
  - (Optional) WiFi library comes with ESP32 core

Install libraries via Arduino Library Manager or PlatformIO.

---

## Setup & Flashing

1. Clone this repository (or copy project files) to your development environment.
2. Open the main sketch in Arduino IDE or PlatformIO.
3. Set the board to an appropriate ESP32 target and select the correct COM port.
4. Configure network credentials and any calibration/thresholds in the sketch:
```cpp
// Example (replace with actual variable names in the sketch)
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";
```
5. Ensure the required libraries are installed (see Software Requirements).
6. Compile and upload to the ESP32.
7. Open Serial Monitor at the configured baud rate to view debug logs and the assigned IP address.

---

## Usage

- After boot, the ESP32 connects to the configured Wi‑Fi and prints the local IP address to the serial monitor and displays status on the LCD.
- Open the device IP address in a browser to access the web dashboard.
- The dashboard fetches sensor data from the endpoint: `/api/data` (polled every 10 seconds by default).
- When an alert condition is met, the LCD and web UI reflect the "SYSTEM ALERT" status, and the buzzer + LED activate.

---

## Calibration
The project applies software offsets to make sensor readings align with local conditions:

- Temperature:
  - Final_Temp = Sensor_Temp - 6.6
- Humidity:
  - Final_Hum = Sensor_Hum - 1.5

Adjust these offsets in code if your local environment or sensors require different calibration.

---

## Alert Logic ("Safety Brain")
Alarm conditions use OR logic: the buzzer and LED activate if ANY of the following are true:

1. High Toxicity: CO sensor value > 1500 (raw/converted PPM depending on implementation)
2. High Heat: Temperature > 23.0 °C
3. Dry Air: Humidity < 40.0 %

Modify thresholds in the code if you need different safety margins.

---

## Troubleshooting

- MQ-7 ADC Saturation (raw reading 4095):
  - A raw value of 4095 on ESP32 ADC often indicates the ADC input is saturated or floating. Verify sensor wiring, common ground, correct output range, and the ADC attenuation settings in code.
  - Ensure MQ-7 analog output is within ESP32 ADC input voltage limits (usually 0–3.3V). If the sensor outputs higher voltages, use a voltage divider or proper driver.
- Web Dashboard Not Loading:
  - Confirm ESP32 connected to Wi‑Fi and note IP in serial monitor.
  - Check that ESPAsyncWebServer and AsyncTCP libraries are correctly installed.
- DHT22 Read Errors:
  - Verify data pin wiring and pull-up resistor (if required).
  - Ensure use of correct library and DHT type in code.
- No Buzzer/LED Response:
  - Confirm GPIO pin assignments match wiring and that outputs are driven with the correct logic levels.
  - For an active buzzer, applying HIGH (or LOW depending on wiring) should trigger the sound—check how your specific buzzer is driven.

---

## Results & Observations
- System connects to Wi‑Fi and displays the device IP on the LCD.
- Dashboard shows live temperature, humidity, and CO readings with color-coded status.
- In normal indoor conditions (~21°C, 60% RH) the system reports "NORMAL".
- When MQ-7 reads a saturated value (e.g., raw 4095), the system enters alert mode — this can indicate sensor saturation, wiring issue, or ADC configuration issue.

---

## Future Improvements
- Mobile app integration (Blynk, MQTT + mobile client)
- Historical data logging (SD card, Google Sheets, or remote database)
- Battery backup (Li-ion + charging circuit) for uninterrupted monitoring
- Improved MQ-7 signal conditioning and calibration to map raw ADC to accurate PPM
- Secure web dashboard (HTTPS / auth) and OTA updates

---

## Contributing
Contributions, bug reports, and improvements are welcome. Please open an issue describing the change or submit a pull request with a clear description of the update and testing notes.

---

## License
This project is provided under the MIT License. See LICENSE file for details.

---

## Contact
Maintainer: farhaislamjitu  
For questions, open an issue in this repository or contact the maintainer directly.
