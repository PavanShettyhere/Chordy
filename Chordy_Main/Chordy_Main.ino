// ============================================================
//  Chordy_Main.ino  —  Entry point & global state machine
//  Arduino IDE: all .ino files in the same folder are compiled
//  together as one translation unit.
// ============================================================

#include "Config.h"

// ── Third-party libraries (install via Library Manager) ──────
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT11.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>        // Install: "ArduinoJson" by Benoit Blanchon
#include <DNSServer.h>
#include <WiFiClientSecure.h>

// ── Global objects ───────────────────────────────────────────
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
DHT11 dht11(DHT_PIN);
WebServer        webServer(WEBSERVER_PORT);
DNSServer        dnsServer;
Preferences      prefs;

// ── Global state ─────────────────────────────────────────────
CConfig      config;
SensorData   sensorData;
WeatherData  weatherData;
RobotState   currentState   = STATE_BOOT;
RobotState   previousState  = STATE_BOOT;
AnimID       currentAnim    = ANIM_IDLE;
AnimID       forcedAnim     = ANIM_COUNT; // ANIM_COUNT = none forced
bool         isAsleep        = false;

// ── Timers ───────────────────────────────────────────────────
unsigned long lastSensorReadMs   = 0;
unsigned long lastWeatherFetchMs = 0;
unsigned long lastLDRSampleMs    = 0;
unsigned long lastAnimTickMs     = 0;
unsigned long lastIdleChangeMs   = 0;
unsigned long timerEndMs         = 0;
bool          timerActive        = false;
int           ldrPrev            = 0;

// ── Button state ─────────────────────────────────────────────
struct BtnState { int pin; bool last; unsigned long lastMs; };
BtnState btns[4] = {
  {BTN_POWER,   HIGH, 0},
  {BTN_SELECT,  HIGH, 0},
  {BTN_INTERACT,HIGH, 0},
  {BTN_EXTRA,   HIGH, 0}
};

// ── Forward declarations (defined in other .ino files) ───────
// Animations.ino
void animInit();
void animTick();
void animPlay(AnimID id);
void buzzerTone(int freq, int durationMs);
void buzzerMelody(int melodyID);

// WebServer.ino
void webServerInit(bool apMode);
void webServerTick();
void dnsServerTick();

// Sensors.ino
void sensorsInit();
void sensorsRead();
void ldrSample();
void pirCheck();

// ── Load config from Preferences ─────────────────────────────
void loadConfig() {
  prefs.begin(PREF_NAMESPACE, true);
  strlcpy(config.botName,  prefs.getString("botName",  "Chordy").c_str(),  32);
  strlcpy(config.wifiSSID, prefs.getString("ssid",     "").c_str(),        64);
  strlcpy(config.wifiPass, prefs.getString("pass",     "").c_str(),        64);
  strlcpy(config.location, prefs.getString("location", "London").c_str(),  64);
  strlcpy(config.owmApiKey,prefs.getString("owmKey",   "").c_str(),        48);
  config.eyeR     = prefs.getUChar("eyeR",  0);
  config.eyeG     = prefs.getUChar("eyeG",  255);
  config.eyeB     = prefs.getUChar("eyeB",  200);
  config.setupDone= prefs.getBool("setup",  false);
  prefs.end();
}

void saveConfig() {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.putString("botName",  config.botName);
  prefs.putString("ssid",     config.wifiSSID);
  prefs.putString("pass",     config.wifiPass);
  prefs.putString("location", config.location);
  prefs.putString("owmKey",   config.owmApiKey);
  prefs.putUChar("eyeR",      config.eyeR);
  prefs.putUChar("eyeG",      config.eyeG);
  prefs.putUChar("eyeB",      config.eyeB);
  prefs.putBool("setup",      config.setupDone);
  prefs.end();
}

static String urlEncode(const char* src) {
  String out;
  const char* hex = "0123456789ABCDEF";
  while (*src) {
    const uint8_t c = (uint8_t)*src++;
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      out += (char)c;
    } else if (c == ' ') {
      out += "%20";
    } else {
      out += '%';
      out += hex[(c >> 4) & 0x0F];
      out += hex[c & 0x0F];
    }
  }
  return out;
}

static bool isRainyWeatherCode(int code) {
  switch (code) {
    case 51: case 53: case 55:
    case 56: case 57:
    case 61: case 63: case 65:
    case 66: case 67:
    case 80: case 81: case 82:
    case 95: case 96: case 99:
      return true;
    default:
      return false;
  }
}

// ── Button polling ────────────────────────────────────────────
void handleButtons() {
  unsigned long now = millis();
  for (int i = 0; i < 4; i++) {
    bool reading = digitalRead(btns[i].pin);
    if (reading != btns[i].last) {
      btns[i].lastMs = now;
    }
    if ((now - btns[i].lastMs) > BTN_DEBOUNCE_MS) {
      if (reading == LOW && btns[i].last == HIGH) {
        // Button pressed
        switch (i) {
          case 0: // BTN_POWER — toggle sleep
            isAsleep = !isAsleep;
            if (isAsleep) {
              currentState = STATE_SLEEPING;
              animPlay(ANIM_SLEEPY);
              buzzerMelody(3);
            } else {
              currentState = STATE_IDLE;
              animPlay(ANIM_IDLE);
              buzzerMelody(0);
            }
            break;
          case 1: // BTN_SELECT — cycle through weather info on display
            if (!isAsleep) {
              animPlay(ANIM_BLINK);
              buzzerTone(1200, 80);
            }
            break;
          case 2: // BTN_INTERACT — pet Chordy
            if (!isAsleep) {
              currentState = STATE_VERY_HAPPY;
              animPlay(ANIM_HEART_EYES);
              buzzerMelody(1);
              lastIdleChangeMs = millis();
            }
            break;
          case 3: // BTN_EXTRA — set/cancel 25-min timer
            if (!isAsleep) {
              if (timerActive) {
                timerActive = false;
                buzzerTone(400, 200);
              } else {
                timerEndMs  = millis() + 25UL * 60UL * 1000UL;
                timerActive = true;
                currentState = STATE_TIMER_RUNNING;
                buzzerMelody(2);
              }
            }
            break;
        }
      }
    }
    btns[i].last = reading;
  }
}

// ── State machine tick ────────────────────────────────────────
void stateMachineTick() {
  unsigned long now = millis();

  // Forced animation from developer panel?
  if (forcedAnim != ANIM_COUNT) {
    animPlay(forcedAnim);
    currentState = STATE_IDLE;
    forcedAnim   = ANIM_COUNT;
  }

  if (isAsleep) return;

  // Timer check
  if (timerActive && now >= timerEndMs) {
    timerActive  = false;
    currentState = STATE_HAPPY;
    animPlay(ANIM_HAPPY);
    buzzerMelody(4); // alarm melody
    lastIdleChangeMs = now;
  }

  // LDR sudden change
  int ldrNow = sensorData.ldrRaw;
  if ((now - lastLDRSampleMs) > LDR_SAMPLE_INTERVAL_MS) {
    int delta = ldrNow - ldrPrev;
    lastLDRSampleMs = now;

    if (delta > LDR_SUDDEN_CHANGE_DELTA) {
      // Sudden bright flash
      currentState = STATE_BLINKING_BRIGHT;
      animPlay(ANIM_SQUINT);
      buzzerTone(600, 100);
      lastIdleChangeMs = now;
    } else if (delta < -LDR_SUDDEN_CHANGE_DELTA) {
      // Sudden darkness
      currentState = STATE_SCARED;
      animPlay(ANIM_SCARED);
      buzzerMelody(5);
      lastIdleChangeMs = now;
    }
    ldrPrev = ldrNow;
  }

  // PIR long absence greeting
  if (sensorData.pirMotion) {
    unsigned long absence = now - sensorData.lastMotionMs;
    if (absence >= PIR_LONG_ABSENCE_MS && currentState == STATE_IDLE) {
      currentState = STATE_VERY_HAPPY;
      animPlay(ANIM_VERY_HAPPY);
      buzzerMelody(1);
      lastIdleChangeMs = now;
    }
  }

  // Weather-based reactions
  if (weatherData.valid && (now - lastIdleChangeMs) > 30000) {
    if (weatherData.tempC >= TEMP_HOT_C && currentState == STATE_IDLE) {
      currentState = STATE_WEATHER_HOT;
      animPlay(ANIM_WEATHER_HOT);
      lastIdleChangeMs = now;
    } else if (weatherData.tempC <= TEMP_COLD_C && currentState == STATE_IDLE) {
      currentState = STATE_WEATHER_COLD;
      animPlay(ANIM_WEATHER_COLD);
      lastIdleChangeMs = now;
    } else if (isRainyWeatherCode(weatherData.conditionCode) && currentState == STATE_IDLE) {
      currentState = STATE_WEATHER_RAINY;
      animPlay(ANIM_WEATHER_RAIN);
      lastIdleChangeMs = now;
    }
  }

  // Return to idle after ~8 seconds of non-idle
  if (currentState != STATE_IDLE &&
      currentState != STATE_SLEEPING &&
      currentState != STATE_TIMER_RUNNING &&
      (now - lastIdleChangeMs) > 8000) {
    currentState = STATE_IDLE;
    // Random idle animation variety
    int r = random(6);
    AnimID idleAnims[] = {ANIM_IDLE, ANIM_BLINK, ANIM_LOOK_LEFT, ANIM_LOOK_RIGHT, ANIM_IDLE, ANIM_BLINK};
    animPlay(idleAnims[r]);
  }

  // Random idle behavior every 5-15 seconds
  if (currentState == STATE_IDLE && (now - lastIdleChangeMs) > (5000 + random(10000))) {
    AnimID idleAnims[] = {ANIM_BLINK, ANIM_LOOK_LEFT, ANIM_LOOK_RIGHT, ANIM_IDLE};
    animPlay(idleAnims[random(4)]);
    lastIdleChangeMs = now;
  }
}

// ── setup() ──────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n[Chordy] Booting...");

  // Buttons
  pinMode(BTN_POWER,    INPUT_PULLUP);
  pinMode(BTN_SELECT,   INPUT_PULLUP);
  pinMode(BTN_INTERACT, INPUT_PULLUP);
  pinMode(BTN_EXTRA,    INPUT_PULLUP);

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[Chordy] OLED init FAILED");
    while (true) delay(1000);
  }

  animInit();
  animPlay(ANIM_STARTUP);
  delay(1500);

  sensorsInit();
  loadConfig();

  if (!config.setupDone || strlen(config.wifiSSID) == 0) {
    // First boot: AP mode
    currentState = STATE_WIFI_SETUP;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("Connect to:");
    display.println(AP_SSID);
    display.println("192.168.4.1");
    display.display();
    webServerInit(true);
  } else {
    // Try to connect to saved WiFi
    currentState = STATE_CONNECTING;
    WiFi.begin(config.wifiSSID, config.wifiPass);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[Chordy] WiFi OK: %s\n", WiFi.localIP().toString().c_str());
      webServerInit(false);
      currentState = STATE_IDLE;
      animPlay(ANIM_HAPPY);
      buzzerMelody(0); // startup jingle
    } else {
      // Fallback to AP mode
      webServerInit(true);
      currentState = STATE_WIFI_SETUP;
    }
  }
  fetchWeather();

  ldrPrev = analogRead(LDR_PIN);
  Serial.println("[Chordy] Ready!");
}

// ── loop() ───────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // Network services
  webServerTick();
  if (currentState == STATE_WIFI_SETUP) dnsServerTick();

  // Sensor reads (non-blocking)
  if (now - lastSensorReadMs > SENSOR_READ_INTERVAL_MS) {
    sensorsRead();
    lastSensorReadMs = now;
  }

  // Weather fetch
  if (WiFi.status() == WL_CONNECTED &&
      now - lastWeatherFetchMs > WEATHER_FETCH_INTERVAL_MS) {
    fetchWeather();
    lastWeatherFetchMs = now;
  }

  // Button handling
  handleButtons();

  // State machine
  stateMachineTick();

  // Animation tick (drives the OLED drawing)
  animTick();
}

// ── fetchWeather() — called from loop ────────────────────────
void fetchWeather() {
  if (strlen(config.location) == 0) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  StaticJsonDocument<2048> doc;

  String geocodeUrl = String("https://") + WEATHER_GEOCODE_HOST + WEATHER_GEOCODE_PATH +
                      "?name=" + urlEncode(config.location) + "&count=1&language=en&format=json";

  Serial.println("[Chordy] Geocode GET: " + geocodeUrl);
  if (!http.begin(client, geocodeUrl)) {
    Serial.println("[Chordy] Geocoding begin failed");
    return;
  }

  int code = http.GET();
  if (code != 200) {
    Serial.printf("[Chordy] Geocoding failed: HTTP %d\n", code);
    http.end();
    return;
  }

  String body = http.getString();
  http.end();

  if (deserializeJson(doc, body)) {
    Serial.println("[Chordy] Geocoding JSON parse failed");
    return;
  }

  JsonArray results = doc["results"].as<JsonArray>();
  if (results.isNull() || results.size() == 0) {
    Serial.printf("[Chordy] No geocoding result for '%s'\n", config.location);
    return;
  }

  const float latitude  = results[0]["latitude"] | 0.0f;
  const float longitude = results[0]["longitude"] | 0.0f;

  doc.clear();
  String url = "https://api.open-meteo.com/v1/forecast?";
  url += "latitude=" + String(latitude, 4);
  url += "&longitude=" + String(longitude, 4);
  url += "&current_weather=true";

  Serial.println("[Chordy] Forecast GET: " + url);
  if (!http.begin(client, url)) {
    Serial.println("[Chordy] Forecast begin failed");
    return;
  }

  code = http.GET();
  if (code != 200) {
    Serial.printf("[Chordy] Forecast failed: HTTP %d\n", code);
    http.end();
    return;
  }

  body = http.getString();
  http.end();

  if (deserializeJson(doc, body)) {
    Serial.println("[Chordy] Forecast JSON parse failed");
    return;
  }

  JsonObject current = doc["current_weather"];
  if (current.isNull()) {
    Serial.println("[Chordy] Forecast response missing current_weather");
    return;
  }

  weatherData.tempC         = current["temperature"] | 0.0f;
  weatherData.windKph       = current["windspeed"] | 0.0f;
  weatherData.conditionCode = current["weathercode"] | 0;
  snprintf(weatherData.description, sizeof(weatherData.description),
           "WMO %d", weatherData.conditionCode);
  weatherData.valid = true;

  Serial.printf("[Chordy] Open-Meteo: %.1f°C  code=%d  (%s)\n",
                weatherData.tempC, weatherData.conditionCode, config.location);
}
