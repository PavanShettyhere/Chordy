# Chordy — Code Structure Guide
## Where to Modify What

This guide is for developers who want to extend or customise Chordy.

---

## File Map

```
Chordy_Main.ino    ← Brain: orchestrates everything
Config.h           ← Constants: pins, timings, data types
Animations.ino     ← Eyes: OLED drawing + buzzer sounds
Sensors.ino        ← Senses: DHT11, LDR, PIR
WebServer.ino      ← Voice: HTTP routes, WiFi manager
WebDashboard.h     ← Face: the web UI (HTML/CSS/JS)
README.md          ← Documentation
docs/circuit_diagram.html    ← Wiring diagram
docs/animation_spec_poster.html  ← Design spec poster
```

---

## 1. Changing GPIO Pins

**File:** `Config.h`

Every GPIO assignment is a `#define` at the top of Config.h:

```cpp
#define DHT_PIN     17   // Change to your pin
#define LDR_PIN     34   // Must be ADC1 (GPIO 32-39) for WiFi compat
#define PIR_PIN     19
#define BUZZER_PIN  18
```

⚠️ LDR must stay on ADC1 (GPIO 32–39). ADC2 stops working when WiFi is active.

---

## 2. Adding a New Animation

**Step 1 — Config.h:** Add your ID to the enum:
```cpp
enum AnimID {
  ...
  ANIM_CONFUSED,   // ← add here
  ANIM_COUNT       // ← always keep this last
};
```

**Step 2 — Animations.ino:** Add a case in `animPlay()`:
```cpp
case ANIM_CONFUSED:
  animFrameDelay = 80;
  animLoop = true;
  break;
```

Add a case in `animTick()`:
```cpp
case ANIM_CONFUSED: {
  // draw squiggly pupils
  int wobble = (animFrame % 6 < 3) ? -2 : 2;
  drawEyes(wobble, 0);
  // draw question mark
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(58, 50);
  display.print("?");
  animFrame++;
  if (animFrame > 60) animPlay(ANIM_IDLE);
  break;
}
```

**Step 3 — WebDashboard.h (optional):** Add a button in the Dev Panel:
```javascript
<button class="btn" onclick="trigger(14)">😕 Confused</button>
```

---

## 3. Adding a New Sensor

**Step 1 — Config.h:** Add a pin define and a field to `SensorData`:
```cpp
#define NEW_SENSOR_PIN  25

struct SensorData {
  ...
  int myNewReading = 0;  // ← add field
};
```

**Step 2 — Sensors.ino:** Read it in `sensorsRead()`:
```cpp
void sensorsRead() {
  ...
  sensorData.myNewReading = analogRead(NEW_SENSOR_PIN);
}
```

**Step 3 — Chordy_Main.ino:** Use it in `stateMachineTick()`:
```cpp
if (sensorData.myNewReading > 2000) {
  animPlay(ANIM_CONFUSED);
}
```

---

## 4. Adding a New Persistent Setting

**Step 1 — Config.h:** Add field to `CConfig`:
```cpp
struct CConfig {
  ...
  bool  myNewFlag = false;
};
```

**Step 2 — Chordy_Main.ino:** Add load/save to `loadConfig()` and `saveConfig()`:
```cpp
// In loadConfig():
config.myNewFlag = prefs.getBool("myFlag", false);

// In saveConfig():
prefs.putBool("myFlag", config.myNewFlag);
```

**Step 3 — WebDashboard.h:** Add a toggle in the dashboard and POST it via `/api/settings`.

**Step 4 — WebServer.ino:** Handle it in `handleSaveSettings()`:
```cpp
if (doc.containsKey("myFlag"))
  config.myNewFlag = doc["myFlag"];
```

---

## 5. Changing Weather Thresholds

**File:** `Config.h`

```cpp
#define TEMP_HOT_C    28.0f   // °C — above this = HOT animation
#define TEMP_COLD_C   15.0f   // °C — below this = COLD animation
```

Add more weather conditions in `stateMachineTick()` in Chordy_Main.ino.
OWM condition codes: https://openweathermap.org/weather-conditions

---

## 6. Changing Buzzer Melodies

**File:** `Animations.ino` → `buzzerMelody()` function

Each melody is an array of `{frequency_hz, duration_ms}`:

```cpp
const Note myMelody[] = {
  {440, 100},   // A4 for 100ms
  {0,   50},    // 50ms silence
  {523, 200},   // C5 for 200ms
};
```

Add your melody to the switch statement and assign it an ID.
To play it: `buzzerMelody(YOUR_ID_NUMBER);`

---

## 7. Adding a New Web API Endpoint

**File:** `WebServer.ino`

**Step 1:** Register the route in `webServerInit()`:
```cpp
webServer.on("/api/myendpoint", HTTP_GET, handleMyEndpoint);
```

**Step 2:** Write the handler:
```cpp
void handleMyEndpoint() {
  char json[128];
  snprintf(json, sizeof(json), "{\"value\":%d}", someValue);
  webServer.send(200, "application/json", json);
}
```

**Step 3 — WebDashboard.h:** Call it from JS:
```javascript
const r = await fetch('/api/myendpoint');
const d = await r.json();
console.log(d.value);
```

---

## 8. Modifying the Web Dashboard

**File:** `WebDashboard.h`

The entire dashboard is a single HTML string in a PROGMEM const.
Edit the HTML/CSS/JS between the `R"HTMLEOF(` and `)HTMLEOF"` delimiters.

ESP32 Flash limit: keep the full HTML under ~100KB.
If it gets large, compress with gzip or split into SPIFFS files.

---

## 9. Changing the PIR Long-Absence Duration

**File:** `Config.h`

```cpp
#define PIR_LONG_ABSENCE_MS   7200000UL   // 2 hours (7,200,000 ms)
// Change to 3 hours:         10800000UL
// Change to 30 minutes:       1800000UL
```

---

## 10. Adjusting Light Sensitivity

**File:** `Config.h`

```cpp
#define LDR_DARK_THRESH       800   // ADC value; above = dark
#define LDR_BRIGHT_THRESH     200   // ADC value; below = very bright
#define LDR_SUDDEN_CHANGE_DELTA 600 // Change in ADC to trigger reaction
```

Lower `LDR_SUDDEN_CHANGE_DELTA` for more sensitivity.
Higher for less sensitivity to gradual changes.

---

## Quick Reference: Extern Variables

These globals are declared in `Chordy_Main.ino` and available everywhere:

| Variable | Type | Description |
|---|---|---|
| `config` | `CConfig` | All persistent settings |
| `sensorData` | `SensorData` | Latest sensor readings |
| `weatherData` | `WeatherData` | Latest weather fetch |
| `currentState` | `RobotState` | Current state machine state |
| `forcedAnim` | `AnimID` | Set to trigger an animation |
| `display` | `Adafruit_SSD1306` | OLED display object |
| `dht` | `DHT` | DHT11 sensor object |
| `webServer` | `WebServer` | HTTP server object |
| `timerActive` | `bool` | Is countdown timer running |
| `timerEndMs` | `unsigned long` | When timer expires (millis) |

---

*Happy hacking! — Chordy is your robot, make it yours.*
