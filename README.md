# 👾 CHORDY — Desk Companion Robot
### Complete Build Guide · Arduino IDE · ESP32

---

## Table of Contents
1. [Overview](#overview)
2. [Hardware Parts List](#hardware-parts-list)
3. [Pin Mapping (Quick Reference)](#pin-mapping)
4. [Wiring Instructions](#wiring-instructions)
5. [Software Setup](#software-setup)
6. [Library Dependencies](#library-dependencies)
7. [First Boot & WiFi Setup](#first-boot--wifi-setup)
8. [Web Dashboard Guide](#web-dashboard-guide)
9. [Animation Reference](#animation-reference)
10. [Buzzer Melodies Reference](#buzzer-melodies-reference)
11. [Button Functions](#button-functions)
12. [Code Structure Guide](#code-structure-guide)
13. [Customization Guide](#customization-guide)
14. [Troubleshooting](#troubleshooting)
15. [FAQ](#faq)

---

## Overview

Chordy is a WiFi-connected desk companion robot powered by an ESP32. It features:

- **Two animated OLED eyes** that react to the environment
- **Environmental sensing** (temperature, humidity, light, motion)
- **Weather integration** via OpenWeatherMap API
- **Sci-Fi cyberpunk web dashboard** for configuration and control
- **Non-blocking animation engine** (no `delay()` used)
- **Captive portal WiFi setup** on first boot
- **Buzzer sound effects** for all interactions

---

## Hardware Parts List

| Component | Quantity | Notes |
|---|---|---|
| ESP32 Dev Board (30-pin) | 1 | WROOM-32 or DevKitC |
| SSD1306 OLED 128×64 (I2C) | 1 | 0.96" or 1.3" module |
| DHT11 Sensor | 1 | With pull-up resistor |
| LDR Sensor Module (4-pin) | 1 | Photodiode + comparator type |
| HC-SR501 PIR Sensor | 1 | Adjustable sensitivity |
| Active Buzzer (3–5V) | 1 | Or passive buzzer for PWM tones |
| Tactile Push Buttons | 4 | 6×6mm, active LOW |
| 10kΩ Resistors | 4 | Button pull-ups (or use internal) |
| Breadboard / PCB | 1 | For prototyping |
| Jumper Wires | ~30 | M-M and M-F |
| USB Micro cable | 1 | For power + programming |
| 3D Printed Body | Optional | See /assets/ for STL concepts |

---

## Pin Mapping

```
┌─────────────────────────────────────────────────────┐
│                   ESP32 Dev Board                    │
│                                                      │
│  GPIO21 ──────── OLED SDA                           │
│  GPIO22 ──────── OLED SCL                           │
│  GPIO17 ──────── DHT11 DATA                         │
│  GPIO34 ──────── LDR Analog Output (ADC1_CH6)       │
│  GPIO19 ──────── PIR Signal Output                  │
│  GPIO18 ──────── Buzzer Positive (+)                │
│  GPIO4  ──────── BTN_POWER (GND when pressed)       │
│  GPIO5  ──────── BTN_SELECT (GND when pressed)      │
│  GPIO13 ──────── BTN_INTERACT (GND when pressed)    │
│  GPIO14 ──────── BTN_EXTRA / Timer (GND when pressed│
│                                                      │
│  3.3V ─┬──────── OLED VCC                          │
│         ├──────── DHT11 VCC                         │
│         └──────── PIR VCC (check module, may be 5V) │
│  GND ──────────── All GND connections               │
│  5V  ──────────── PIR VCC (if HC-SR501)             │
└─────────────────────────────────────────────────────┘
```

> ⚠️ **GPIO34 is input-only** on ESP32 — perfect for ADC. Do NOT connect output signals here.
> ⚠️ **HC-SR501 PIR** typically requires 5V. Use the ESP32's 5V (VIN) pin or an external supply.

---

## Wiring Instructions

### OLED Display (SSD1306)
```
OLED VCC  → ESP32 3.3V
OLED GND  → ESP32 GND
OLED SDA  → ESP32 GPIO21
OLED SCL  → ESP32 GPIO22
```

### DHT11 Sensor
```
DHT11 VCC  → ESP32 3.3V
DHT11 GND  → ESP32 GND
DHT11 DATA → ESP32 GPIO17
           [10kΩ pull-up from DATA to 3.3V — built into most modules]
```

### LDR Module (4-pin module)
```
LDR VCC    → ESP32 3.3V
LDR GND    → ESP32 GND
LDR AO     → ESP32 GPIO34   (Analog output for brightness value)
LDR DO     → Not connected  (Digital comparator output, not used)
```

### PIR Sensor (HC-SR501)
```
PIR VCC    → ESP32 5V (VIN pin)
PIR GND    → ESP32 GND
PIR OUT    → ESP32 GPIO19
```

### Buzzer
```
Buzzer (+) → ESP32 GPIO18
Buzzer (-) → ESP32 GND
```
*For a passive buzzer: GPIO18 drives PWM directly. For active buzzer: GPIO18 HIGH = sound.*

### Buttons (all Active LOW with internal pull-up)
```
BTN_POWER    → One leg to GPIO4,  other leg to GND
BTN_SELECT   → One leg to GPIO5,  other leg to GND
BTN_INTERACT → One leg to GPIO13, other leg to GND
BTN_EXTRA    → One leg to GPIO14, other leg to GND
```

---

## Software Setup

### 1. Install Arduino IDE
Download from: https://www.arduino.cc/en/software (version 2.x recommended)

### 2. Add ESP32 Board Support
- Open `File → Preferences`
- Add to "Additional Boards Manager URLs":
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- Open `Tools → Board → Boards Manager`
- Search "esp32" by Espressif Systems → Install (version 2.x)

### 3. Select Board Settings
```
Board:    "ESP32 Dev Module"
Upload Speed: 921600
CPU Frequency: 240MHz
Flash Size: 4MB
Partition Scheme: Default 4MB with spiffs
Port: (your COM port)
```

### 4. Open the Project
- In Arduino IDE: `File → Open → Chordy_Main.ino`
- All `.ino` files in the same folder are auto-included

---

## Library Dependencies

Install all via `Sketch → Include Library → Manage Libraries`:

| Library | Author | Version |
|---|---|---|
| Adafruit SSD1306 | Adafruit | ≥ 2.5.7 |
| Adafruit GFX Library | Adafruit | ≥ 1.11.9 |
| DHT sensor library | Adafruit | ≥ 1.4.6 |
| ArduinoJson | Benoit Blanchon | ≥ 7.x |

All other libraries (`WiFi`, `WebServer`, `Preferences`, `HTTPClient`, `DNSServer`) are **included with ESP32 Arduino Core**.

---

## First Boot & WiFi Setup

1. **Power on Chordy** — the OLED will show startup eyes animation
2. Chordy creates a WiFi hotspot: **`Chordy_Setup`** (open, no password)
3. Connect your phone/laptop to `Chordy_Setup`
4. A captive portal opens automatically (or navigate to `192.168.4.1`)
5. Fill in:
   - **Bot Name**: e.g. `Chordy`
   - **WiFi SSID**: Your home/office network name
   - **WiFi Password**: Your network password
   - **Location**: City name for weather (e.g. `London`)
   - **OWM API Key**: Free key from openweathermap.org (optional)
6. Click **INITIALIZE CHORDY**
7. Chordy saves settings to flash, restarts, and connects automatically

> Credentials are stored in ESP32 NVS (non-volatile storage). Chordy remembers them across power cycles.

---

## Web Dashboard Guide

Once connected to WiFi, open `http://<chordy-ip>` in any browser.

Find Chordy's IP address in Serial Monitor (115200 baud) after boot, or check your router's DHCP table.

### Dashboard Sections:

**Internal Sensors Panel**
- Live temperature (DHT11) and humidity readings
- Ambient light bar (LDR) — left = dark, right = bright
- PIR motion indicator (green dot = motion detected)

**External Weather Panel**
- Current temperature and conditions from OpenWeatherMap
- Wind speed in km/h
- Condition code (used internally to select weather animations)

**Eye Preview**
- Real-time canvas preview of eye colour
- Eyes animate in the preview (blink, look around)

**Eye Colour Picker**
- RGB sliders for custom eye colour
- Preview updates live
- Click **APPLY** to save to Chordy

**Focus Timer**
- Set a Pomodoro-style countdown (default 25 min)
- Progress bar + MM:SS display
- Chordy plays alarm melody when timer ends

**Configuration**
- Change Bot Name (shown at OLED bottom)
- Change location for weather
- Update OpenWeatherMap API key

**⚡ Developer Panel**
- Directly trigger any animation on Chordy for testing
- 12 animations available — all run immediately on the robot

---

## Animation Reference

| ID | Name | Description | Trigger |
|---|---|---|---|
| 0 | IDLE | Gentle eyes, occasional blink | Default state |
| 1 | BLINK | Single smooth blink | Random / BTN_SELECT |
| 2 | HAPPY | Bouncing pupils + hands waving | After connection, timer done |
| 3 | VERY_HAPPY | Heart eyes + bounce | After 2h absence, petting |
| 4 | SCARED | Wide eyes + screen shake | Sudden darkness |
| 5 | SLEEPY | Half-lid eyes + ZZZ symbols | Sleep mode, BTN_POWER |
| 6 | SQUINT | Closing eyes | Sudden bright flash |
| 7 | HEART_EYES | Hearts in eyes | BTN_INTERACT |
| 8 | LOOK_LEFT | Pupils slide left | Random idle |
| 9 | LOOK_RIGHT | Pupils slide right | Random idle |
| 10 | WEATHER_COLD | Shivering eyes + blanket | Temp ≤ 15°C |
| 11 | WEATHER_HOT | Sweating + spinning fan | Temp ≥ 28°C |
| 12 | WEATHER_RAIN | Rain drops + cloud overlay | Rain weather code |
| 13 | STARTUP | Eyes grow from center | Boot sequence |

---

## Buzzer Melodies Reference

| ID | Name | When Played |
|---|---|---|
| 0 | Startup Jingle | WiFi connected / boot |
| 1 | Happy Tune | Petting / Very Happy state |
| 2 | Timer Set | BTN_EXTRA pressed (set timer) |
| 3 | Sleep Tune | BTN_POWER (enter sleep) |
| 4 | Timer Alarm | Countdown reaches zero |
| 5 | Scared Sound | Sudden darkness detected |

---

## Button Functions

| Button | GPIO | Single Press | Hold (2s+) |
|---|---|---|---|
| BTN_POWER | 4 | Toggle sleep/wake | - |
| BTN_SELECT | 5 | Cycle display / blink | - |
| BTN_INTERACT | 13 | Pet Chordy → Heart Eyes | - |
| BTN_EXTRA | 14 | Start/cancel 25-min timer | - |

---

## Code Structure Guide

```
Chordy_Main.ino     ← Main entry point
│  setup()          — Hardware init, WiFi connect, first-boot check
│  loop()           — Orchestrates all modules each tick
│  stateMachineTick() — Decides what state/animation to show
│  handleButtons()  — Debounced button handler
│  loadConfig()     — Read settings from NVS
│  saveConfig()     — Write settings to NVS
│  fetchWeather()   — HTTP call to OpenWeatherMap

Config.h            ← All constants and data structures
│  Pin definitions  — Change GPIO numbers here
│  RobotState enum  — Add new states here
│  AnimID enum      — Add new animation IDs here
│  CConfig struct   — Add new persistent settings here

Animations.ino      ← OLED drawing engine
│  animPlay(id)     — Switch to a new animation
│  animTick()       — Called every loop() — advances frames
│  drawEyes(...)    — Core eye renderer
│  drawBlanket()    — Weather overlay: cold
│  drawFan()        — Weather overlay: hot
│  drawRain()       — Weather overlay: rain
│  buzzerTone()     — Single tone
│  buzzerMelody()   — Play a melody sequence

Sensors.ino         ← Sensor abstraction
│  sensorsInit()    — pinMode and dht.begin()
│  sensorsRead()    — Read DHT11, LDR, PIR into sensorData struct

WebServer.ino       ← HTTP server
│  webServerInit()  — Route registration, WiFi mode
│  handleSetup()    — AP captive portal HTML
│  handleDashboard()— Full sci-fi dashboard
│  handleTelemetry()— GET /api/telemetry → JSON
│  handleTrigger()  — POST /api/trigger → force animation
│  handleSetTimer() — POST /api/timer → set/cancel timer
│  handleSaveSettings() — POST /api/settings → update config

WebDashboard.h      ← 700-line HTML/CSS/JS dashboard
```

### Where To Modify What

| Goal | File | What to change |
|---|---|---|
| Change GPIO pins | `Config.h` | `#define` constants |
| Add a new animation | `Animations.ino` + `Config.h` | Add case to `animTick()`, add to `AnimID` enum |
| Add a new sensor | `Sensors.ino` + `Config.h` | Add to `sensorsRead()`, add field to `SensorData` |
| Add a new state | `Config.h` + `Chordy_Main.ino` | Add to `RobotState` enum, add handling in `stateMachineTick()` |
| Change weather thresholds | `Config.h` | `TEMP_HOT_C`, `TEMP_COLD_C` |
| Change LDR sensitivity | `Config.h` | `LDR_DARK_THRESH`, `LDR_SUDDEN_CHANGE_DELTA` |
| Change absence time before happy | `Config.h` | `PIR_LONG_ABSENCE_MS` |
| Add a web dashboard feature | `WebDashboard.h` | Add HTML/JS, add endpoint in `WebServer.ino` |
| Add persistent setting | `Config.h` (CConfig) + `Chordy_Main.ino` (load/save) | Add field, load/save in prefs |
| Change buzzer melodies | `Animations.ino` | Edit `Note` arrays in `buzzerMelody()` |

---

## Troubleshooting

**OLED not working**
- Check I2C address: some modules use 0x3D. Scan with I2C scanner sketch
- Verify SDA=21, SCL=22, VCC=3.3V
- Check `Wire.begin(21, 22)` is called before `display.begin()`

**DHT11 reads NaN**
- Ensure 10kΩ pull-up between DATA and VCC
- DHT11 needs ~1 second between reads — already handled
- Try a longer cable for DATA (add capacitor if interference)

**WiFi won't connect**
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- SSID/password case-sensitive
- Reset config: hold BTN_POWER on boot (TODO: add to enhancement list)

**LDR not responding**
- GPIO34 is ADC1_CH6 — works with analogRead() when WiFi is active
- ADC2 pins do NOT work when WiFi is active — this is why we use GPIO34
- Check AO pin on module (not DO)

**Web server not loading**
- Find IP in Serial Monitor after boot
- Try `http://` not `https://`
- Some browsers block HTTP on local networks — use Chrome or Firefox

**Buzzer always on / wrong pitch**
- Ensure `ledcWrite(0, 0)` is called at end of melody
- If using active buzzer: use `digitalWrite` instead of `ledcWriteTone`

---

## FAQ

**Q: Can I use a different OLED size?**  
A: Yes. Change `OLED_WIDTH` and `OLED_HEIGHT` in Config.h and adjust eye positions accordingly.

**Q: Can I power Chordy from a battery?**  
A: Yes. Use a 3.7V LiPo with a TP4056 charging module + 3.3V LDO regulator, or a USB power bank.

**Q: How do I get an OpenWeatherMap API key?**  
A: Register free at openweathermap.org → API → Current Weather Data → Free tier (60 calls/min).

**Q: Can Chordy work without internet?**  
A: Yes. All animations, sensors, and the web server work offline. Only weather integration requires internet.

**Q: How do I reset Chordy to factory defaults?**  
A: Upload the sketch with `config.setupDone = false` and `saveConfig()` in setup() to force AP mode.

**Q: Can I add more animations?**  
A: Yes! See "Where to Modify What" section. Add your AnimID to the enum, add a case in `animTick()`, and call `animPlay(YOUR_ID)` wherever you want to trigger it.

---

## Credits

- Built with ❤️ using Arduino IDE + ESP32
- OLED rendering: Adafruit_SSD1306 + Adafruit_GFX
- JSON parsing: ArduinoJson by Benoit Blanchon
- Weather data: OpenWeatherMap API
- Project name: **Chordy** — your desk friend

---

*Last updated: 2026 · Open Source — modify freely*
