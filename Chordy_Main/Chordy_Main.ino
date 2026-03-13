// ============================================================
//  Chordy_Main.ino  —  v2.0
//  Fixed buttons, screen cycling, factory reset, NTP time
// ============================================================
#include "Config.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT11.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WiFiClientSecure.h>
#include <time.h>

// ── Global objects ───────────────────────────────────────────
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
DHT11            dht11(DHT_PIN);
WebServer        webServer(WEBSERVER_PORT);
DNSServer        dnsServer;
Preferences      prefs;

// ── Global state ─────────────────────────────────────────────
CConfig       config;
SensorData    sensorData;
WeatherData   weatherData;
TimeData      timeData;
RobotState    currentState  = STATE_BOOT;
AnimID        currentAnim   = ANIM_IDLE;
AnimID        forcedAnim    = ANIM_COUNT;
bool          isAsleep      = false;
DisplayScreen currentScreen = SCREEN_FACE;

// ── Timers ───────────────────────────────────────────────────
unsigned long lastSensorReadMs    = 0;
unsigned long lastWeatherFetchMs  = 0;
unsigned long lastLDRSampleMs     = 0;
unsigned long lastIdleChangeMs    = 0;
unsigned long timerEndMs          = 0;
unsigned long timerTotalMs        = 0;
unsigned long ldrReactionEndMs    = 0;
bool          timerActive         = false;
bool          inLdrReaction       = false;
int           ldrPrev             = 0;

// ── Button state (FIXED: proper edge detection) ──────────────
struct BtnState {
  int           pin;
  bool          currentReading;
  bool          lastReading;
  bool          pressed;        // true for one cycle when pressed
  unsigned long debounceMs;
};
BtnState btns[4] = {
  {BTN_POWER,    HIGH, HIGH, false, 0},
  {BTN_SELECT,   HIGH, HIGH, false, 0},
  {BTN_INTERACT, HIGH, HIGH, false, 0},
  {BTN_EXTRA,    HIGH, HIGH, false, 0},
};

// ── Forward decls ────────────────────────────────────────────
void animInit();
void animTick();
void animPlay(AnimID id);
void buzzerTone(int freq, int durationMs);
void buzzerMelody(int melodyID);
void webServerInit(bool apMode);
void webServerTick();
void dnsServerTick();
void sensorsInit();
void sensorsRead();
void fetchWeather();
void fetchTime();
void handleDashboard();

// ── Config persistence ────────────────────────────────────────
void loadConfig() {
  prefs.begin(PREF_NAMESPACE, true);
  strlcpy(config.botName,  prefs.getString("botName",  "Chordy").c_str(), 32);
  strlcpy(config.wifiSSID, prefs.getString("ssid",     "").c_str(),       64);
  strlcpy(config.wifiPass, prefs.getString("pass",     "").c_str(),       64);
  strlcpy(config.location, prefs.getString("location", "Dortmund").c_str(),64);
  config.eyeR          = prefs.getUChar("eyeR",     0);
  config.eyeG          = prefs.getUChar("eyeG",     255);
  config.eyeB          = prefs.getUChar("eyeB",     200);
  config.eyeStyle      = prefs.getUChar("eyeStyle", 0);
  config.animSpeed     = prefs.getUChar("animSpd",  5);
  config.buzzerEnabled = prefs.getBool ("buzzerOn", true);
  config.buzzerVolume  = prefs.getUChar("buzVol",   7);
  config.setupDone     = prefs.getBool ("setup",    false);
  prefs.end();
}

void saveConfig() {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.putString("botName",  config.botName);
  prefs.putString("ssid",     config.wifiSSID);
  prefs.putString("pass",     config.wifiPass);
  prefs.putString("location", config.location);
  prefs.putUChar("eyeR",      config.eyeR);
  prefs.putUChar("eyeG",      config.eyeG);
  prefs.putUChar("eyeB",      config.eyeB);
  prefs.putUChar("eyeStyle",  config.eyeStyle);
  prefs.putUChar("animSpd",   config.animSpeed);
  prefs.putBool ("buzzerOn",  config.buzzerEnabled);
  prefs.putUChar("buzVol",    config.buzzerVolume);
  prefs.putBool ("setup",     config.setupDone);
  prefs.end();
}

void factoryReset() {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.clear();
  prefs.end();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 24);
  display.println("Factory Reset...");
  display.display();
  delay(1500);
  ESP.restart();
}

// ── URL encode ────────────────────────────────────────────────
static String urlEncode(const char* src) {
  String out;
  const char* hex = "0123456789ABCDEF";
  while (*src) {
    const uint8_t c = (uint8_t)*src++;
    if ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~')
      out += (char)c;
    else if (c==' ') out += "%20";
    else { out += '%'; out += hex[(c>>4)&0x0F]; out += hex[c&0x0F]; }
  }
  return out;
}

static bool isRainyCode(int c) {
  return (c>=51&&c<=67)||(c>=80&&c<=82)||(c>=95&&c<=99);
}

// ── Button polling — FIXED ─────────────────────────────────────
void pollButtons() {
  unsigned long now = millis();
  for (int i = 0; i < 4; i++) {
    btns[i].pressed = false;
    int raw = digitalRead(btns[i].pin);
    if (raw != btns[i].lastReading) {
      btns[i].debounceMs = now;
    }
    if ((now - btns[i].debounceMs) > BTN_DEBOUNCE_MS) {
      if (raw != btns[i].currentReading) {
        btns[i].currentReading = raw;
        if (raw == LOW) {      // just pressed (active LOW)
          btns[i].pressed = true;
        }
      }
    }
    btns[i].lastReading = raw;
  }
}

void handleButtons() {
  for (int i = 0; i < 4; i++) {
    if (!btns[i].pressed) continue;

    switch (i) {
      case 0: // BTN_POWER — toggle sleep
        isAsleep = !isAsleep;
        if (isAsleep) {
          currentState = STATE_SLEEPING;
          currentScreen = SCREEN_FACE;
          animPlay(ANIM_SLEEPY);
          if (config.buzzerEnabled) buzzerMelody(3);
        } else {
          currentState = STATE_IDLE;
          animPlay(ANIM_IDLE);
          if (config.buzzerEnabled) buzzerMelody(0);
        }
        break;

      case 1: // BTN_SELECT — cycle display screens
        if (!isAsleep) {
          currentScreen = (DisplayScreen)((currentScreen + 1) % SCREEN_COUNT);
          if (config.buzzerEnabled) buzzerTone(1000, 60);
          switch (currentScreen) {
            case SCREEN_FACE:    currentState = STATE_IDLE; animPlay(ANIM_BLINK); break;
            case SCREEN_CLOCK:   currentState = STATE_CLOCK; break;
            case SCREEN_WEATHER: currentState = STATE_WEATHER_INFO; break;
            case SCREEN_SENSORS: currentState = STATE_SENSORS_INFO; break;
            default: break;
          }
          lastIdleChangeMs = millis();
        }
        break;

      case 2: // BTN_INTERACT — pet Chordy
        if (!isAsleep) {
          currentScreen = SCREEN_FACE;
          currentState = STATE_VERY_HAPPY;
          animPlay(ANIM_HEART_EYES);
          if (config.buzzerEnabled) buzzerMelody(1);
          lastIdleChangeMs = millis();
        }
        break;

      case 3: // BTN_EXTRA — start/cancel timer (default 25 min)
        if (!isAsleep) {
          currentScreen = SCREEN_FACE;
          if (timerActive) {
            timerActive = false;
            if (config.buzzerEnabled) buzzerTone(400, 200);
          } else {
            timerTotalMs = 25UL * 60UL * 1000UL;
            timerEndMs   = millis() + timerTotalMs;
            timerActive  = true;
            currentState = STATE_TIMER_RUNNING;
            if (config.buzzerEnabled) buzzerMelody(2);
          }
        }
        break;
    }
  }
}

// ── Display screen renderers ──────────────────────────────────
void renderClockScreen() {
  display.clearDisplay();
  // Get current time (NTP synced or estimated)
  struct tm ti;
  if (getLocalTime(&ti)) {
    char timeBuf[12], dateBuf[20];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
    snprintf(dateBuf, sizeof(dateBuf), "%02d/%02d/%04d", ti.tm_mday, ti.tm_mon+1, ti.tm_year+1900);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    int tw = strlen(timeBuf)*12;
    display.setCursor((128-tw)/2, 12);
    display.print(timeBuf);
    display.setTextSize(1);
    int dw = strlen(dateBuf)*6;
    display.setCursor((128-dw)/2, 36);
    display.print(dateBuf);
  } else {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("No time data");
    display.setCursor(10, 34);
    display.println("Check WiFi");
  }
  // Location
  display.setTextSize(1);
  display.setCursor(2, 55);
  display.print(config.location);
  // Screen label
  display.setCursor(0, 0);
  display.print("[CLOCK]");
  display.display();
}

void renderWeatherScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 0);
  display.print("[WEATHER] ");
  display.print(config.location);
  if (weatherData.valid) {
    display.setCursor(2, 12);
    display.printf("Temp: %.1fC", weatherData.tempC);
    display.setCursor(2, 22);
    display.printf("Wind: %.1f km/h", weatherData.windKph);
    display.setCursor(2, 32);
    display.printf("Code: %d", weatherData.conditionCode);
    display.setCursor(2, 42);
    display.print(weatherData.description);
    // Condition emoji text
    display.setCursor(2, 54);
    if (weatherData.tempC >= TEMP_HOT_C)      display.print("Hot! Stay cool :)");
    else if (weatherData.tempC <= TEMP_COLD_C) display.print("Cold! Stay warm :)");
    else                                       display.print("Nice weather!");
  } else {
    display.setCursor(2, 24);
    display.println("Fetching weather...");
    display.setCursor(2, 36);
    display.print("(needs WiFi + loc)");
  }
  display.display();
}

void renderSensorsScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 0);  display.print("[SENSORS]");
  display.setCursor(2, 12); display.printf("Int Temp: %.1f C", sensorData.tempC);
  display.setCursor(2, 22); display.printf("Humidity: %.0f %%", sensorData.humidity);
  display.setCursor(2, 32); display.printf("Light LDR: %d", sensorData.ldrRaw);
  display.setCursor(2, 42); display.print("PIR: ");
  display.print(sensorData.pirMotion ? "Motion!" : "Clear");
  // Light bar
  int barW = map(constrain(4095 - sensorData.ldrRaw, 0, 4095), 0, 4095, 0, 80);
  display.drawRect(44, 54, 82, 7, SSD1306_WHITE);
  display.fillRect(44, 54, barW, 7, SSD1306_WHITE);
  display.setCursor(2, 55); display.print("Lux:");
  display.display();
}

// ── State machine tick ────────────────────────────────────────
void stateMachineTick() {
  unsigned long now = millis();

  // Forced anim from web
  if (forcedAnim != ANIM_COUNT) {
    animPlay(forcedAnim);
    forcedAnim = ANIM_COUNT;
    currentState = STATE_IDLE;
    lastIdleChangeMs = now;
  }

  if (isAsleep) return;

  // Non-face screens handled by their renderers, skip face logic
  if (currentScreen != SCREEN_FACE) return;

  // Timer completion
  if (timerActive && now >= timerEndMs) {
    timerActive = false;
    currentState = STATE_HAPPY;
    animPlay(ANIM_HAPPY);
    if (config.buzzerEnabled) buzzerMelody(4);
    lastIdleChangeMs = now;
  }

  // LDR sudden change (with timed duration)
  int ldrNow = sensorData.ldrRaw;
  if ((now - lastLDRSampleMs) > LDR_SAMPLE_INTERVAL_MS) {
    int delta = ldrNow - ldrPrev;
    lastLDRSampleMs = now;

    if (!inLdrReaction) {
      if (delta > LDR_SUDDEN_CHANGE_DELTA) {
        currentState = STATE_BLINKING_BRIGHT;
        animPlay(ANIM_SQUINT);
        if (config.buzzerEnabled) buzzerTone(600, 80);
        inLdrReaction = true;
        ldrReactionEndMs = now + LDR_REACTION_DURATION_MS;
        lastIdleChangeMs = now;
      } else if (delta < -LDR_SUDDEN_CHANGE_DELTA) {
        currentState = STATE_SCARED;
        animPlay(ANIM_SCARED);
        if (config.buzzerEnabled) buzzerMelody(5);
        inLdrReaction = true;
        ldrReactionEndMs = now + LDR_REACTION_DURATION_MS;
        lastIdleChangeMs = now;
      }
    }
    ldrPrev = ldrNow;
  }

  // LDR reaction timeout — return to idle
  if (inLdrReaction && now >= ldrReactionEndMs) {
    inLdrReaction = false;
    currentState = STATE_IDLE;
    animPlay(ANIM_IDLE);
    lastIdleChangeMs = now;
    return;
  }
  if (inLdrReaction) return;

  // PIR long absence greeting
  if (sensorData.pirMotion) {
    unsigned long absence = now - sensorData.lastMotionMs;
    if (absence >= PIR_LONG_ABSENCE_MS && currentState == STATE_IDLE) {
      currentState = STATE_VERY_HAPPY;
      animPlay(ANIM_VERY_HAPPY);
      if (config.buzzerEnabled) buzzerMelody(1);
      lastIdleChangeMs = now;
    }
  }

  // Weather reactions (only while idle, max once every 30s)
  if (weatherData.valid && currentState == STATE_IDLE &&
      (now - lastIdleChangeMs) > 30000) {
    if (weatherData.tempC >= TEMP_HOT_C) {
      currentState = STATE_WEATHER_HOT;
      animPlay(ANIM_WEATHER_HOT);
      lastIdleChangeMs = now;
    } else if (weatherData.tempC <= TEMP_COLD_C) {
      currentState = STATE_WEATHER_COLD;
      animPlay(ANIM_WEATHER_COLD);
      lastIdleChangeMs = now;
    } else if (isRainyCode(weatherData.conditionCode)) {
      currentState = STATE_WEATHER_RAINY;
      animPlay(ANIM_WEATHER_RAIN);
      lastIdleChangeMs = now;
    }
  }

  // Return to idle after timeout (non-idle states)
  if (currentState != STATE_IDLE &&
      currentState != STATE_SLEEPING &&
      currentState != STATE_TIMER_RUNNING &&
      (now - lastIdleChangeMs) > 8000) {
    currentState = STATE_IDLE;
    AnimID back[] = {ANIM_IDLE, ANIM_BLINK, ANIM_LOOK_LEFT, ANIM_LOOK_RIGHT};
    animPlay(back[random(4)]);
    lastIdleChangeMs = now;
  }

  // Rich idle behaviour — new random emotions pool
  if (currentState == STATE_IDLE &&
      (now - lastIdleChangeMs) > (IDLE_ANIM_MIN_MS + random(IDLE_ANIM_MAX_EXTRA_MS))) {
    AnimID idlePool[] = {
      ANIM_BLINK, ANIM_LOOK_LEFT, ANIM_LOOK_RIGHT, ANIM_LOOK_UP, ANIM_LOOK_DOWN,
      ANIM_WINK, ANIM_THINKING, ANIM_IDLE, ANIM_BLINK,
      ANIM_PLAYING_GUITAR, ANIM_WHISTLING, ANIM_COFFEE, ANIM_TYPING
    };
    int n = sizeof(idlePool)/sizeof(idlePool[0]);
    animPlay(idlePool[random(n)]);
    lastIdleChangeMs = now;
  }
}

// ── setup() ──────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n[Chordy v2] Booting...");

  // Buttons with explicit INPUT_PULLUP
  pinMode(BTN_POWER,    INPUT_PULLUP);
  pinMode(BTN_SELECT,   INPUT_PULLUP);
  pinMode(BTN_INTERACT, INPUT_PULLUP);
  pinMode(BTN_EXTRA,    INPUT_PULLUP);

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[Chordy] OLED FAILED");
    while (true) delay(1000);
  }
  display.setTextColor(SSD1306_WHITE);

  animInit();
  animPlay(ANIM_STARTUP);
  delay(1500);

  sensorsInit();
  loadConfig();

  if (!config.setupDone || strlen(config.wifiSSID) == 0) {
    currentState = STATE_WIFI_SETUP;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);  display.println("=== SETUP MODE ===");
    display.setCursor(0, 12); display.println("WiFi: Chordy_Setup");
    display.setCursor(0, 24); display.println("Go to 192.168.4.1");
    display.setCursor(0, 36); display.println("in your browser");
    display.display();
    webServerInit(true);
  } else {
    currentState = STATE_CONNECTING;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20); display.print("Connecting WiFi...");
    display.display();

    WiFi.begin(config.wifiSSID, config.wifiPass);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 20) {
      delay(500); tries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[Chordy] WiFi OK: %s\n", WiFi.localIP().toString().c_str());
      // NTP sync
      configTime(0, 0, "pool.ntp.org", "time.nist.gov");
      webServerInit(false);
      fetchWeather();
      currentState = STATE_IDLE;
      animPlay(ANIM_HAPPY);
      if (config.buzzerEnabled) buzzerMelody(0);
    } else {
      Serial.println("[Chordy] WiFi failed, starting AP mode");
      webServerInit(true);
      currentState = STATE_WIFI_SETUP;
    }
  }

  ldrPrev = analogRead(LDR_PIN);
  Serial.println("[Chordy v2] Ready!");
}

// ── loop() ───────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // Highest priority: buttons first
  pollButtons();
  handleButtons();

  webServerTick();
  if (currentState == STATE_WIFI_SETUP) dnsServerTick();

  if (now - lastSensorReadMs > SENSOR_READ_INTERVAL_MS) {
    sensorsRead();
    lastSensorReadMs = now;
  }

  if (WiFi.status() == WL_CONNECTED &&
      now - lastWeatherFetchMs > WEATHER_FETCH_INTERVAL_MS) {
    fetchWeather();
    lastWeatherFetchMs = now;
  }


  stateMachineTick();

  // Screen dispatch
  switch (currentScreen) {
    case SCREEN_FACE:    animTick(); break;
    case SCREEN_CLOCK:   renderClockScreen(); delay(400); break;
    case SCREEN_WEATHER: renderWeatherScreen(); delay(600); break;
    case SCREEN_SENSORS: renderSensorsScreen(); delay(500); break;
    default: animTick(); break;
  }
}

// ── fetchWeather() ────────────────────────────────────────────
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED || strlen(config.location) == 0) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String geocodeUrl = String("https://") + WEATHER_GEOCODE_HOST + WEATHER_GEOCODE_PATH +
                      "?name=" + urlEncode(config.location) + "&count=1&language=en&format=json";
  if (!http.begin(client, geocodeUrl)) return;
  int code = http.GET();
  if (code != 200) { http.end(); return; }
  String body = http.getString();
  http.end();

  StaticJsonDocument<1024> doc;
  if (deserializeJson(doc, body)) return;
  JsonArray res = doc["results"];
  if (res.isNull() || res.size() == 0) return;

  float lat = res[0]["latitude"]  | 0.0f;
  float lon = res[0]["longitude"] | 0.0f;

  doc.clear();
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(lat,4) +
               "&longitude=" + String(lon,4) + "&current_weather=true";
  if (!http.begin(client, url)) return;
  code = http.GET();
  if (code != 200) { http.end(); return; }
  body = http.getString();
  http.end();

  if (deserializeJson(doc, body)) return;
  JsonObject cur = doc["current_weather"];
  if (cur.isNull()) return;

  weatherData.tempC         = cur["temperature"] | 0.0f;
  weatherData.windKph       = cur["windspeed"]   | 0.0f;
  weatherData.conditionCode = cur["weathercode"] | 0;
  snprintf(weatherData.description, sizeof(weatherData.description),
           "WMO code %d", weatherData.conditionCode);
  weatherData.valid = true;
  Serial.printf("[Weather] %.1f°C code=%d\n", weatherData.tempC, weatherData.conditionCode);
}
