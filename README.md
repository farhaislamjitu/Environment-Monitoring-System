# Environment Monitoring System

A simple, extensible environment monitoring system for collecting, visualizing, and alerting on sensor data from IoT devices (temperature, humidity, air quality, etc.). Designed for hobbyists, researchers, and small deployments.

## Features

- Collects data from multiple sensors (DHT22/BME280/PM2.5, etc.)
- Local storage (SQLite) and optional remote upload (MQTT/HTTP)
- Web dashboard for real-time visualization
- Configurable alerting (email, SMS, webhook)
- Extensible: add new sensors or backends via a plugin interface

## Architecture

- Devices (Raspberry Pi / ESP32) run a lightweight client that reads sensors and pushes data
- Backend server stores, processes, and serves data to the dashboard
- Dashboard provides charts, device status, and alert configuration

## Getting Started

### Prerequisites

- Python 3.8+ (for backend and dashboard)
- Node.js 16+ (only if the dashboard is a separate frontend)
- Git

### Installation (Backend)

1. Clone the repository:

   git clone https://github.com/farhaislamjitu/Environment-Monitoring-System.git
   cd Environment-Monitoring-System

2. Create and activate a virtual environment:

   python -m venv venv
   source venv/bin/activate   # Linux / macOS
   venv\Scripts\activate    # Windows

3. Install dependencies:

   pip install -r requirements.txt

4. Configure environment variables or edit config/config.yaml (example provided):

   - DATABASE_URL: sqlite:///data/envmon.db
   - MQTT_BROKER: mqtt.example.com
   - ALERT_EMAIL: you@example.com

5. Initialize the database (if applicable):

   python manage.py init-db

6. Run the backend:

   python manage.py run

### Running Dashboard (if present)

If the frontend is a separate app (check /frontend):

   cd frontend
   npm install
   npm run dev

Open http://localhost:3000 (or the port shown) in your browser.

## Device Client (Raspberry Pi / ESP32)

- See /devices for sample scripts and configuration for Raspberry Pi (Python) and ESP32 (Arduino/PlatformIO).
- Each device reads sensors and sends JSON payloads to the backend via MQTT or HTTP POST.

## Configuration

See config/config.example.yaml for example settings. Key sections:

- sensors: list of enabled sensors and GPIO/pin settings
- storage: local or remote (sqlite, postgres, cloud)
- alerts: thresholds and notification backends

## Development

- Contribute using feature branches and PRs.
- Run tests with:

   pytest

- Follow the code style (black, flake8) where configured.

## Contributing

Contributions are welcome. Open issues or PRs, and follow the contribution guidelines in CONTRIBUTING.md (if present).

## License

Specify a license (e.g., MIT) in LICENSE.md.

## Contact

Maintainer: farhaislamjitu
