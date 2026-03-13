# 👾 CHORDY v2.0 — Desk Companion Robot
### Complete Build Guide · Arduino IDE · ESP32

---

## What's New in v2.0

| Feature | Status |
|---|---|
| ✅ Fixed button debounce (physical buttons now work correctly) | FIXED |
| ✅ BTN_SELECT cycles Face → Clock → Weather → Sensors screen | NEW |
| ✅ Improved arms/hands drawing (proper shoulder/elbow/hand) | FIXED |
| ✅ Mouth expressions (smile/frown) added to emotions | FIXED |
| ✅ Animation speed slider in web dashboard (1–10) | NEW |
| ✅ Timer now has seconds input as well as minutes | NEW |
| ✅ Shrinking border around OLED during timer countdown | NEW |
| ✅ LDR reactions limited to 3–5 seconds then return to idle | FIXED |
| ✅ Rich idle animations: thinking, wink, look up/down, activities | NEW |
| ✅ New emotions: Guitar, Whistle, Fly-tracking, Typing, Coffee, Magnify | NEW |
| ✅ Buzzer sounds for all new emotions | NEW |
| ✅ Buzzer toggle (on/off) in web dashboard | NEW |
| ✅ Buzzer volume slider in web dashboard | NEW |
| ✅ Eye style picker: Round, Oval, Cute (anime), Wide | NEW |
| ✅ Virtual button panel on web dashboard | NEW |
| ✅ Factory reset button (forgets all WiFi + settings) | NEW |
| ✅ NTP time sync — real clock display | NEW |
| ✅ Clock screen on OLED (via BTN_SELECT) | NEW |
| ✅ Chordy Emotion Builder tool (standalone HTML app) | NEW |

---

## File Structure

```
Chordy_Main/
├── Chordy_Main.ino    ← Main entry, state machine, button fix, screen cycling
├── Animations.ino     ← All eye drawing, new emotions, buzzer, timer border
├── WebServer.ino      ← HTTP routes, virtual buttons, factory reset API
├── WebDashboard.h     ← Complete sci-fi dashboard HTML/CSS/JS
├── Config.h           ← Pins, enums, structs (EyeStyle, DisplayScreen)
└── Sensors.ino        ← DHT11, LDR, PIR

Docs/
├── Chordy_EmotionBuilder.html  ← Standalone pixel editor + code generator
├── circuit_diagram.html
├── animation_spec_poster.html
└── CODE_GUIDE.md
```

---

## Hardware

Same as v1.0 — no hardware changes required.

| Component | GPIO | Notes |
|---|---|---|
| OLED SDA | 21 | I2C |
| OLED SCL | 22 | I2C |
| DHT11 | 17 | |
| LDR AO | 34 | ADC1 only |
| PIR | 19 | HC-SR501 on 5V |
| Buzzer | 18 | PWM/passive |
| BTN_POWER | 4 | Sleep toggle |
| BTN_SELECT | 5 | Cycle screens |
| BTN_INTERACT | 13 | Pet Chordy |
| BTN_EXTRA | 14 | Timer |

---

## Library Requirements

Same as v1.0 plus **time.h** (included in ESP32 Arduino core):

| Library | Notes |
|---|---|
| Adafruit SSD1306 | ≥ 2.5 |
| Adafruit GFX | ≥ 1.11 |
| DHT11 library | Any Arduino-compatible |
| ArduinoJson | v7.x by Benoit Blanchon |

**NTP time sync** uses the built-in `configTime()` / `getLocalTime()` — no extra library needed.

---

## Button Functions (v2)

| Button | GPIO | Function |
|---|---|---|
| BTN_POWER | 4 | Toggle sleep / wake |
| BTN_SELECT | 5 | Cycle OLED screen: FACE → CLOCK → WEATHER → SENSORS |
| BTN_INTERACT | 13 | Pet Chordy → Heart eyes + happy sound |
| BTN_EXTRA | 14 | Start/cancel 25-minute focus timer |

### Why buttons weren't working (v1 bug)
The v1 code had a logic error: it tested `reading == LOW && btns[i].last == HIGH` **before** updating `btns[i].last`, so the last reading was never updated in the right place. v2 uses proper 3-state debounce: raw read → debounce → stable read → edge detect.

---

## OLED Screens

Press **BTN_SELECT** (or "SCREEN" virtual button in web) to cycle:

1. **FACE** — Animated eyes, current emotion, timer border
2. **CLOCK** — Large HH:MM:SS, date, location (NTP synced)
3. **WEATHER** — Temperature, wind, condition, advice
4. **SENSORS** — Internal temp/humidity, LDR bar, PIR status

---

## Web Dashboard Features (v2)

Open `http://<chordy-ip>` in any browser.

### Virtual Buttons
Replicate all 4 physical buttons from the web.

### Screen Control
Tap any screen pill (FACE / CLOCK / WEATHER / SENSORS) to switch the OLED instantly.

### Eye Style
Choose from Round (default), Oval (tall), Cute (anime), Wide. Saved to flash.

### Animation Speed
Slider 1–10. Affects all animation frame delays proportionally. Saved to flash.

### Audio
- Toggle buzzer on/off
- Volume slider (maps to PWM duty cycle, 1–10)
- Both saved to flash

### Focus Timer
- Minutes + Seconds inputs
- Shrinking border on OLED shows remaining time
- Plays alarm melody when done

### Developer Panel
Trigger any of 24 animations directly:
- Basic emotions (blink, happy, scared, sleepy, squint, wink, hearts...)
- Activities (guitar, whistle, fly, typing, coffee, magnify, thinking)
- Weather (cold, hot, rain)

### Factory Reset
Clears all NVS storage and restarts Chordy in setup mode. Use when changing WiFi network.

---

## New Animations

| Name | Description |
|---|---|
| GUITAR | Plays air guitar, musical notes float up, guitar sound |
| WHISTLING | Puffy cheeks, mouth O, notes fly right, whistle melody |
| FLY | Eyes track a moving fly, question mark blinks |
| TYPING | Eyes flicker left-right, keyboard drawn at bottom |
| COFFEE | Steam rises from cup, contented half-lid eyes |
| MAGNIFY | One eye enlarged with magnifying glass overlay |
| THINKING | Eyes look up-right, thought bubble with ? or ! |
| WINK | Right eye closes and reopens with easing |
| LOOK_UP | Pupils slide up |
| LOOK_DOWN | Pupils slide down |

---

## Chordy Emotion Builder

Open `Docs/Chordy_EmotionBuilder.html` in any modern browser (no server needed — works offline).

### Features:
- 128×64 pixel grid editor (matches OLED resolution exactly)
- Multi-frame timeline with copy/paste/duplicate
- 6 eye presets to start from
- Shift/invert/fill tools
- Animation playback at adjustable FPS
- Buzzer melody builder with note names
- **Generates complete Arduino code** to paste into Animations.ino
- Integration guide shows exactly where to paste each code block

### Workflow:
1. Design frames in the pixel editor
2. Configure name, speed, loop, sound
3. Click "Generate Code"
4. Copy and paste into the 5 marked spots in the firmware
5. Flash to ESP32

---

## LDR Reaction Timing (v2 fix)

**v1 problem:** LDR reactions (scared/squint) would loop indefinitely.

**v2 fix:** `LDR_REACTION_DURATION_MS = 4000` in Config.h. After 4 seconds the emotion ends and Chordy returns to idle. Change this value to adjust duration:

```cpp
#define LDR_REACTION_DURATION_MS  4000UL  // 4 seconds
```

---

## Eye Styles

| Style | Description | EYE_W | EYE_H |
|---|---|---|---|
| ROUND (0) | Default, balanced | 26 | 20 |
| OVAL (1) | Taller, expressive | 22 | 26 |
| CUTE (2) | Anime-style round | 20 | 20 |
| WIDE (3) | Wide, surprised look | 30 | 18 |

Change from the web dashboard or add `eyeStyle = X` to setup page.

---

## NTP Time Configuration

Time zone is set to UTC by default. To change, edit in `Chordy_Main.ino`:

```cpp
// UTC (default)
configTime(0, 0, "pool.ntp.org");

// UTC+1 (Germany / CET)
configTime(3600, 3600, "pool.ntp.org");

// UTC+5:30 (India / IST)
configTime(19800, 0, "pool.ntp.org");
```

First arg = UTC offset in seconds. Second = DST offset.

---

## Troubleshooting (v2)

**Buttons still not working:**
- Verify `INPUT_PULLUP` is set (it is in v2 setup())
- Check GPIO numbers match your wiring and Config.h
- Try Serial Monitor — button presses print debug if `Serial.println` added
- Some ESP32 dev boards have GPIO 12 strapping issue — avoid if problems

**Sound is too quiet / too loud:**
- Adjust `buzzerVolume` slider in web dashboard
- Passive buzzers need PWM (GPIO 18 is LEDC capable) — works as-is
- Active buzzers: may need `digitalWrite` instead of `ledcWriteTone` — swap in `buzzerMelody()`

**Clock shows "--:--:--":**
- Requires WiFi connection for NTP sync
- NTP sync takes ~5 seconds after WiFi connects
- Check `configTime()` timezone offset in setup()

**Factory reset:**
- Use the Factory Reset button in the web dashboard Danger Zone
- OR hold any button during boot and add a check in setup() to call `factoryReset()`

---

*Chordy v2.0 · ESP32 · Arduino IDE · Open Source*
