// ============================================================
//  WebServer.ino  —  Setup portal + dashboard API
// ============================================================

#include "Config.h"
#include "WebDashboard.h"

extern CConfig     config;
extern SensorData  sensorData;
extern WeatherData weatherData;
extern RobotState  currentState;
extern AnimID      forcedAnim;
extern UIScreen    currentScreen;
extern bool        timerActive;
extern bool        isAsleep;
extern unsigned long timerEndMs;
extern unsigned long timerDurationMs;

static bool _apMode = false;

void saveConfig();
void handleButtonPress(int index);
void setTimerForMs(unsigned long durationMs);

void handleRoot();
void handleSetup();
void handleSave();
void handleTelemetry();
void handleSaveSettings();
void handleTrigger();
void handleSetTimer();
void handleControl();
void handleNotFound();
void handleDashboard();

void webServerInit(bool apMode) {
  _apMode = apMode;

  if (apMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    dnsServer.start(53, "*", WiFi.softAPIP());
  } else {
    WiFi.mode(WIFI_STA);
  }

  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/setup", HTTP_GET, handleSetup);
  webServer.on("/save", HTTP_POST, handleSave);
  webServer.on("/api/telemetry", HTTP_GET, handleTelemetry);
  webServer.on("/api/settings", HTTP_POST, handleSaveSettings);
  webServer.on("/api/trigger", HTTP_POST, handleTrigger);
  webServer.on("/api/timer", HTTP_POST, handleSetTimer);
  webServer.on("/api/control", HTTP_POST, handleControl);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
}

void webServerTick() { webServer.handleClient(); }
void dnsServerTick() { dnsServer.processNextRequest(); }

void handleSetup() {
  webServer.send(200, "text/html", R"rawhtml(
<!DOCTYPE html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Chordy Setup</title>
<style>body{background:#07131d;color:#d8faff;font-family:Arial,sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh}
.card{background:#0d1c28;border:1px solid #2f6070;border-radius:14px;padding:24px;width:min(420px,92vw)}input{width:100%;padding:10px;margin:8px 0 14px;background:#08141d;color:#d8faff;border:1px solid #2f6070;border-radius:8px}button{width:100%;padding:12px;background:#154458;color:#d8faff;border:1px solid #2ea0bb;border-radius:8px}</style>
</head><body><div class="card"><h2>Chordy Setup</h2>
<form action="/save" method="POST">
<label>Bot Name</label><input name="name" value="Chordy">
<label>WiFi SSID</label><input name="ssid">
<label>WiFi Password</label><input type="password" name="pass">
<label>Location</label><input name="location" value="Dortmund">
<button type="submit">Save and restart</button>
</form></div></body></html>)rawhtml");
}

void handleRoot() {
  if (_apMode) { handleSetup(); return; }
  handleDashboard();
}

void handleSave() {
  if (!webServer.hasArg("ssid")) {
    webServer.send(400, "text/plain", "Bad request");
    return;
  }

  strlcpy(config.wifiSSID, webServer.arg("ssid").c_str(), 64);
  strlcpy(config.wifiPass, webServer.arg("pass").c_str(), 64);
  strlcpy(config.botName, webServer.arg("name").c_str(), 32);
  strlcpy(config.location, webServer.arg("location").c_str(), 64);
  config.setupDone = true;
  saveConfig();

  webServer.send(200, "text/html", "<html><body style='font-family:Arial;background:#07131d;color:#d8faff;display:flex;align-items:center;justify-content:center;height:100vh'>Saved. Restarting...</body></html>");
  delay(1200);
  ESP.restart();
}

void handleTelemetry() {
  char json[768];
  unsigned long remaining = 0;
  if (timerActive && timerEndMs > millis()) remaining = (timerEndMs - millis()) / 1000UL;

  snprintf(json, sizeof(json),
    "{"
    "\"temp_c\":%.1f,"
    "\"humidity\":%.0f,"
    "\"ldr\":%d,"
    "\"pir\":%s,"
    "\"weather_temp\":%.1f,"
    "\"weather_wind\":%.1f,"
    "\"weather_desc\":\"%s\","
    "\"weather_code\":%d,"
    "\"weather_valid\":%s,"
    "\"state\":%d,"
    "\"screen\":%d,"
    "\"timer_active\":%s,"
    "\"timer_remaining\":%lu,"
    "\"botName\":\"%s\","
    "\"location\":\"%s\","
    "\"eyeR\":%u,"
    "\"eyeG\":%u,"
    "\"eyeB\":%u,"
    "\"eyeStyle\":%u,"
    "\"animSpeedPct\":%u,"
    "\"buzzerEnabled\":%s,"
    "\"buzzerVolume\":%u"
    "}",
    sensorData.tempC, sensorData.humidity, sensorData.ldrRaw,
    sensorData.pirMotion ? "true" : "false",
    weatherData.tempC, weatherData.windKph, weatherData.description,
    weatherData.conditionCode,
    weatherData.valid ? "true" : "false",
    (int)currentState, (int)currentScreen,
    timerActive ? "true" : "false",
    remaining,
    config.botName, config.location,
    config.eyeR, config.eyeG, config.eyeB,
    config.eyeStyle, config.animSpeedPct,
    config.buzzerEnabled ? "true" : "false",
    config.buzzerVolume
  );
  webServer.send(200, "application/json", json);
}

void handleSaveSettings() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  StaticJsonDocument<320> doc;
  if (deserializeJson(doc, webServer.arg("plain"))) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  if (doc.containsKey("botName"))  strlcpy(config.botName, doc["botName"], 32);
  if (doc.containsKey("location")) strlcpy(config.location, doc["location"], 64);
  if (doc.containsKey("eyeR")) config.eyeR = constrain((int)doc["eyeR"], 0, 255);
  if (doc.containsKey("eyeG")) config.eyeG = constrain((int)doc["eyeG"], 0, 255);
  if (doc.containsKey("eyeB")) config.eyeB = constrain((int)doc["eyeB"], 0, 255);
  if (doc.containsKey("eyeStyle")) config.eyeStyle = constrain((int)doc["eyeStyle"], 0, 2);
  if (doc.containsKey("animSpeedPct")) config.animSpeedPct = constrain((int)doc["animSpeedPct"], 40, 180);
  if (doc.containsKey("buzzerEnabled")) config.buzzerEnabled = doc["buzzerEnabled"];
  if (doc.containsKey("buzzerVolume")) config.buzzerVolume = constrain((int)doc["buzzerVolume"], 0, 100);
  saveConfig();

  webServer.send(200, "application/json", "{\"ok\":true}");
}

void handleTrigger() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  StaticJsonDocument<64> doc;
  if (deserializeJson(doc, webServer.arg("plain"))) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  int animId = doc["anim"] | 0;
  if (animId >= 0 && animId < ANIM_COUNT) {
    forcedAnim = (AnimID)animId;
    currentScreen = SCREEN_FACE;
    webServer.send(200, "application/json", "{\"ok\":true}");
    return;
  }
  webServer.send(400, "application/json", "{\"ok\":false}");
}

void handleSetTimer() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  StaticJsonDocument<96> doc;
  if (deserializeJson(doc, webServer.arg("plain"))) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  int mins = max(0, (int)(doc["minutes"] | 0));
  int secs = max(0, (int)(doc["seconds"] | 0));
  unsigned long totalMs = ((unsigned long)mins * 60UL + (unsigned long)secs) * 1000UL;
  setTimerForMs(totalMs);
  webServer.send(200, "application/json", "{\"ok\":true}");
}

void handleControl() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  StaticJsonDocument<96> doc;
  if (deserializeJson(doc, webServer.arg("plain"))) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  String action = doc["action"] | "";
  if (action == "power") handleButtonPress(0);
  else if (action == "select") handleButtonPress(1);
  else if (action == "interact") handleButtonPress(2);
  else if (action == "extra") handleButtonPress(3);
  else if (action == "face") currentScreen = SCREEN_FACE;
  else if (action == "menu") currentScreen = SCREEN_MENU;
  else if (action == "settings") currentScreen = SCREEN_SETTINGS;
  else {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  webServer.send(200, "application/json", "{\"ok\":true}");
}

void handleNotFound() {
  if (_apMode) { handleSetup(); return; }
  webServer.send(404, "text/plain", "404 Not Found");
}

void handleDashboard() {
  webServer.send_P(200, "text/html", DASHBOARD_HTML);
}
