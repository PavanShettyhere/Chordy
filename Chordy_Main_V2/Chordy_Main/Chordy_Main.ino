// ============================================================
//  Chordy_Main.ino  —  Entry point & global state machine
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

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
DHT11            dht11(DHT_PIN);
WebServer        webServer(WEBSERVER_PORT);
DNSServer        dnsServer;
Preferences      prefs;

CConfig      config;
SensorData   sensorData;
WeatherData  weatherData;
RobotState   currentState  = STATE_BOOT;
RobotState   previousState = STATE_BOOT;
AnimID       currentAnim   = ANIM_IDLE;
AnimID       forcedAnim    = ANIM_COUNT;
UIScreen     currentScreen = SCREEN_FACE;
bool         isAsleep      = false;

unsigned long lastSensorReadMs   = 0;
unsigned long lastWeatherFetchMs = 0;
unsigned long lastLDRSampleMs    = 0;
unsigned long lastIdleChangeMs   = 0;
unsigned long timerEndMs         = 0;
unsigned long timerDurationMs    = 0;
bool          timerActive        = false;
int           ldrPrev            = 0;

struct BtnState {
  int pin;
  bool stableState;
  bool lastReading;
  unsigned long lastChangeMs;
};

BtnState btns[4] = {
  {BTN_POWER,   HIGH, HIGH, 0},
  {BTN_SELECT,  HIGH, HIGH, 0},
  {BTN_INTERACT,HIGH, HIGH, 0},
  {BTN_EXTRA,   HIGH, HIGH, 0}
};

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

void setTimerForMs(unsigned long durationMs) {
  if (durationMs == 0) {
    timerActive = false;
    timerDurationMs = 0;
    timerEndMs = 0;
    if (currentState == STATE_TIMER_RUNNING) currentState = STATE_IDLE;
    return;
  }
  timerDurationMs = durationMs;
  timerEndMs = millis() + durationMs;
  timerActive = true;
  currentState = STATE_TIMER_RUNNING;
}

void loadConfig() {
  prefs.begin(PREF_NAMESPACE, true);
  strlcpy(config.botName,  prefs.getString("botName",  "Chordy").c_str(),  32);
  strlcpy(config.wifiSSID, prefs.getString("ssid",     "").c_str(),        64);
  strlcpy(config.wifiPass, prefs.getString("pass",     "").c_str(),        64);
  strlcpy(config.location, prefs.getString("location", "Dortmund").c_str(),64);
  strlcpy(config.owmApiKey,prefs.getString("owmKey",   "").c_str(),        48);
  config.eyeR          = prefs.getUChar("eyeR", 0);
  config.eyeG          = prefs.getUChar("eyeG", 255);
  config.eyeB          = prefs.getUChar("eyeB", 200);
  config.eyeStyle      = prefs.getUChar("eyeStyle", 0);
  config.buzzerVolume  = prefs.getUChar("buzzVol", 70);
  config.animSpeedPct  = prefs.getUChar("animSpd", 100);
  config.buzzerEnabled = prefs.getBool("buzzOn", true);
  config.setupDone     = prefs.getBool("setup", false);
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
  prefs.putUChar("eyeStyle",  config.eyeStyle);
  prefs.putUChar("buzzVol",   config.buzzerVolume);
  prefs.putUChar("animSpd",   config.animSpeedPct);
  prefs.putBool("buzzOn",     config.buzzerEnabled);
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
    case 51: case 53: case 55: case 56: case 57:
    case 61: case 63: case 65: case 66: case 67:
    case 80: case 81: case 82: case 95: case 96: case 99:
      return true;
    default:
      return false;
  }
}

static void toggleScreen() {
  currentScreen = (UIScreen)((currentScreen + 1) % SCREEN_COUNT);
}

void handleButtonPress(int index) {
  switch (index) {
    case 0: // power
      isAsleep = !isAsleep;
      if (isAsleep) {
        currentState = STATE_SLEEPING;
        currentScreen = SCREEN_FACE;
        animPlay(ANIM_SLEEPY);
        buzzerMelody(3);
      } else {
        currentState = STATE_IDLE;
        animPlay(ANIM_HAPPY);
        buzzerMelody(0);
        lastIdleChangeMs = millis();
      }
      break;

    case 1: // select: toggle among face/menu/settings
      if (!isAsleep) {
        toggleScreen();
        buzzerTone(1100, 40);
      }
      break;

    case 2: // interact
      if (isAsleep) break;
      if (currentScreen == SCREEN_SETTINGS) {
        config.eyeStyle = (config.eyeStyle + 1) % 3;
        saveConfig();
        buzzerTone(1400, 50);
      } else {
        currentState = STATE_VERY_HAPPY;
        currentScreen = SCREEN_FACE;
        animPlay(ANIM_HEART_EYES);
        buzzerMelody(1);
        lastIdleChangeMs = millis();
      }
      break;

    case 3: // timer / confirm
      if (isAsleep) break;
      if (timerActive) {
        setTimerForMs(0);
        buzzerTone(350, 120);
      } else {
        setTimerForMs(25UL * 60UL * 1000UL);
        buzzerMelody(2);
      }
      currentScreen = SCREEN_FACE;
      break;
  }
}

void handleButtons() {
  unsigned long now = millis();
  for (int i = 0; i < 4; ++i) {
    bool reading = digitalRead(btns[i].pin);

    if (reading != btns[i].lastReading) {
      btns[i].lastChangeMs = now;
      btns[i].lastReading = reading;
    }

    if ((now - btns[i].lastChangeMs) > BTN_DEBOUNCE_MS && reading != btns[i].stableState) {
      btns[i].stableState = reading;
      if (btns[i].stableState == LOW) {
        handleButtonPress(i);
      }
    }
  }
}

void stateMachineTick() {
  unsigned long now = millis();

  if (forcedAnim != ANIM_COUNT) {
    currentScreen = SCREEN_FACE;
    animPlay(forcedAnim);
    currentState = STATE_IDLE;
    forcedAnim = ANIM_COUNT;
  }

  if (isAsleep) return;

  if (timerActive && now >= timerEndMs) {
    timerActive = false;
    timerDurationMs = 0;
    currentState = STATE_HAPPY;
    currentScreen = SCREEN_FACE;
    animPlay(ANIM_HAPPY);
    buzzerMelody(4);
    lastIdleChangeMs = now;
  }

  int ldrNow = sensorData.ldrRaw;
  if ((now - lastLDRSampleMs) > LDR_SAMPLE_INTERVAL_MS) {
    int delta = ldrNow - ldrPrev;
    lastLDRSampleMs = now;

    if (delta > LDR_SUDDEN_CHANGE_DELTA) {
      currentState = STATE_BLINKING_BRIGHT;
      currentScreen = SCREEN_FACE;
      animPlay(ANIM_SQUINT);
      buzzerTone(550, 70);
      lastIdleChangeMs = now;
    } else if (delta < -LDR_SUDDEN_CHANGE_DELTA) {
      currentState = STATE_SCARED;
      currentScreen = SCREEN_FACE;
      animPlay(ANIM_SCARED);
      buzzerMelody(5);
      lastIdleChangeMs = now;
    }
    ldrPrev = ldrNow;
  }

  if (sensorData.pirMotion) {
    unsigned long absence = now - sensorData.lastMotionMs;
    if (absence >= PIR_LONG_ABSENCE_MS && currentState == STATE_IDLE) {
      currentState = STATE_VERY_HAPPY;
      currentScreen = SCREEN_FACE;
      animPlay(ANIM_VERY_HAPPY);
      buzzerMelody(1);
      lastIdleChangeMs = now;
    }
  }

  if (weatherData.valid && (now - lastIdleChangeMs) > 30000) {
    if (weatherData.tempC >= TEMP_HOT_C && currentState == STATE_IDLE) {
      currentState = STATE_WEATHER_HOT;
      currentScreen = SCREEN_FACE;
      animPlay(ANIM_WEATHER_HOT);
      buzzerMelody(6);
      lastIdleChangeMs = now;
    } else if (weatherData.tempC <= TEMP_COLD_C && currentState == STATE_IDLE) {
      currentState = STATE_WEATHER_COLD;
      currentScreen = SCREEN_FACE;
      animPlay(ANIM_WEATHER_COLD);
      buzzerMelody(7);
      lastIdleChangeMs = now;
    } else if (isRainyWeatherCode(weatherData.conditionCode) && currentState == STATE_IDLE) {
      currentState = STATE_WEATHER_RAINY;
      currentScreen = SCREEN_FACE;
      animPlay(ANIM_WEATHER_RAIN);
      buzzerMelody(8);
      lastIdleChangeMs = now;
    }
  }

  if (currentState != STATE_IDLE &&
      currentState != STATE_SLEEPING &&
      currentState != STATE_TIMER_RUNNING &&
      (now - lastIdleChangeMs) > 8000) {
    currentState = STATE_IDLE;
    AnimID idleAnims[] = {ANIM_IDLE, ANIM_BLINK, ANIM_LOOK_LEFT, ANIM_LOOK_RIGHT, ANIM_IDLE, ANIM_BLINK};
    animPlay(idleAnims[random(6)]);
  }

  if (currentState == STATE_IDLE && currentScreen == SCREEN_FACE && (now - lastIdleChangeMs) > (5000 + random(7000))) {
    AnimID idleAnims[] = {ANIM_BLINK, ANIM_LOOK_LEFT, ANIM_LOOK_RIGHT, ANIM_IDLE};
    animPlay(idleAnims[random(4)]);
    lastIdleChangeMs = now;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n[Chordy] Booting...");

  pinMode(BTN_POWER,    INPUT_PULLUP);
  pinMode(BTN_SELECT,   INPUT_PULLUP);
  pinMode(BTN_INTERACT, INPUT_PULLUP);
  pinMode(BTN_EXTRA,    INPUT_PULLUP);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[Chordy] OLED init FAILED");
    while (true) delay(1000);
  }

  animInit();
  animPlay(ANIM_STARTUP);
  delay(1000);

  sensorsInit();
  loadConfig();

  randomSeed(analogRead(LDR_PIN) + millis());

  if (!config.setupDone || strlen(config.wifiSSID) == 0) {
    currentState = STATE_WIFI_SETUP;
    webServerInit(true);
  } else {
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
      buzzerMelody(0);
    } else {
      webServerInit(true);
      currentState = STATE_WIFI_SETUP;
    }
  }

  fetchWeather();
  ldrPrev = analogRead(LDR_PIN);
  lastIdleChangeMs = millis();
  Serial.println("[Chordy] Ready!");
}

void loop() {
  unsigned long now = millis();

  webServerTick();
  if (currentState == STATE_WIFI_SETUP) dnsServerTick();

  if (now - lastSensorReadMs > SENSOR_READ_INTERVAL_MS) {
    sensorsRead();
    lastSensorReadMs = now;
  }

  if (WiFi.status() == WL_CONNECTED && now - lastWeatherFetchMs > WEATHER_FETCH_INTERVAL_MS) {
    fetchWeather();
    lastWeatherFetchMs = now;
  }

  handleButtons();
  stateMachineTick();
  animTick();
}

void fetchWeather() {
  if (strlen(config.location) == 0) return;
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  StaticJsonDocument<2048> doc;

  String geocodeUrl = String("https://") + WEATHER_GEOCODE_HOST + WEATHER_GEOCODE_PATH +
                      "?name=" + urlEncode(config.location) + "&count=1&language=en&format=json";

  if (!http.begin(client, geocodeUrl)) return;
  int code = http.GET();
  if (code != 200) {
    http.end();
    return;
  }

  String body = http.getString();
  http.end();
  if (deserializeJson(doc, body)) return;

  JsonArray results = doc["results"].as<JsonArray>();
  if (results.isNull() || results.size() == 0) return;

  const float latitude  = results[0]["latitude"] | 0.0f;
  const float longitude = results[0]["longitude"] | 0.0f;

  doc.clear();
  String url = "https://api.open-meteo.com/v1/forecast?";
  url += "latitude=" + String(latitude, 4);
  url += "&longitude=" + String(longitude, 4);
  url += "&current_weather=true";

  if (!http.begin(client, url)) return;
  code = http.GET();
  if (code != 200) {
    http.end();
    return;
  }

  body = http.getString();
  http.end();
  if (deserializeJson(doc, body)) return;

  JsonObject current = doc["current_weather"];
  if (current.isNull()) return;

  weatherData.tempC         = current["temperature"] | 0.0f;
  weatherData.windKph       = current["windspeed"] | 0.0f;
  weatherData.conditionCode = current["weathercode"] | 0;
  snprintf(weatherData.description, sizeof(weatherData.description), "WMO %d", weatherData.conditionCode);
  weatherData.valid = true;
}
