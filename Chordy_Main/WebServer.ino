// ============================================================
//  WebServer.ino  —  WiFi Manager (AP captive portal) +
//                    Sci-Fi/Cyberpunk ESP32 Web Dashboard
// ============================================================

#include "Config.h"
#include "WebDashboard.h"

extern CConfig     config;
extern SensorData  sensorData;
extern WeatherData weatherData;
extern RobotState  currentState;
extern AnimID      forcedAnim;
extern bool        timerActive;
extern unsigned long timerEndMs;

static bool _apMode = false;

// ── Forward decls ─────────────────────────────────────────────
void handleRoot();
void handleSetup();
void handleSave();
void handleTelemetry();
void handleSettings();
void handleSaveSettings();
void handleTrigger();
void handleSetTimer();
void handleNotFound();

// ──────────────────────────────────────────────────────────────
void webServerInit(bool apMode) {
  _apMode = apMode;

  if (apMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    dnsServer.start(53, "*", WiFi.softAPIP());
    Serial.printf("[Web] AP mode — %s  IP: %s\n",
                  AP_SSID, WiFi.softAPIP().toString().c_str());
  } else {
    WiFi.mode(WIFI_STA);
    Serial.printf("[Web] STA mode — IP: %s\n",
                  WiFi.localIP().toString().c_str());
  }

  // Routes
  webServer.on("/",          HTTP_GET,  handleRoot);
  webServer.on("/setup",     HTTP_GET,  handleSetup);
  webServer.on("/save",      HTTP_POST, handleSave);
  webServer.on("/api/telemetry",     HTTP_GET,  handleTelemetry);
  webServer.on("/api/settings",      HTTP_POST, handleSaveSettings);
  webServer.on("/api/trigger",       HTTP_POST, handleTrigger);
  webServer.on("/api/timer",         HTTP_POST, handleSetTimer);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  Serial.println("[Web] HTTP server started");
}

void webServerTick() { webServer.handleClient(); }
void dnsServerTick() { dnsServer.processNextRequest(); }

// ──────────────────────────────────────────────────────────────
// AP Setup Portal
// ──────────────────────────────────────────────────────────────
void handleSetup() {
  webServer.send(200, "text/html", R"rawhtml(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Chordy Setup</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&family=Share+Tech+Mono&display=swap');
  *{box-sizing:border-box;margin:0;padding:0}
  body{background:#030912;color:#00ffe7;font-family:'Share Tech Mono',monospace;min-height:100vh;display:flex;align-items:center;justify-content:center}
  .card{background:linear-gradient(135deg,#030d1a,#061525);border:1px solid #00ffe740;border-radius:16px;padding:40px 32px;width:100%;max-width:420px;box-shadow:0 0 40px #00ffe720}
  h1{font-family:'Orbitron',sans-serif;font-size:1.8rem;color:#00ffe7;text-shadow:0 0 20px #00ffe780;margin-bottom:8px;text-align:center}
  .sub{color:#00ffe780;text-align:center;font-size:.85rem;margin-bottom:32px}
  label{display:block;font-size:.8rem;color:#00ffe7aa;margin-bottom:6px;margin-top:20px}
  input{width:100%;background:#061525;border:1px solid #00ffe740;border-radius:8px;padding:12px 16px;color:#00ffe7;font-family:'Share Tech Mono',monospace;font-size:.95rem;outline:none;transition:border-color .2s}
  input:focus{border-color:#00ffe7}
  button{margin-top:28px;width:100%;background:linear-gradient(90deg,#00ffe720,#00ffe740);border:1px solid #00ffe7;border-radius:8px;padding:14px;color:#00ffe7;font-family:'Orbitron',sans-serif;font-size:1rem;font-weight:700;letter-spacing:2px;cursor:pointer;text-transform:uppercase;transition:all .2s}
  button:hover{background:linear-gradient(90deg,#00ffe740,#00ffe760);box-shadow:0 0 20px #00ffe740}
  .eye-icon{text-align:center;font-size:2rem;margin-bottom:8px}
</style></head><body>
<div class="card">
  <div class="eye-icon">👾</div>
  <h1>CHORDY</h1>
  <p class="sub">Initial Configuration</p>
  <form action="/save" method="POST">
    <label>Bot Name</label>
    <input name="name" placeholder="Chordy" value="Chordy">
    <label>WiFi Network (SSID)</label>
    <input name="ssid" placeholder="Your WiFi Name">
    <label>WiFi Password</label>
    <input name="pass" type="password" placeholder="••••••••">
    <label>Location (for weather)</label>
    <input name="location" placeholder="London">
    <label>OpenWeatherMap API Key</label>
    <input name="owmkey" placeholder="(optional)">
    <button type="submit">▶ INITIALIZE CHORDY</button>
  </form>
</div></body></html>
)rawhtml");
}

void handleRoot() {
  if (_apMode) { handleSetup(); return; }
  // Serve the full sci-fi dashboard
  handleDashboard();
}

void handleSave() {
  if (webServer.hasArg("ssid")) {
    strlcpy(config.wifiSSID, webServer.arg("ssid").c_str(), 64);
    strlcpy(config.wifiPass, webServer.arg("pass").c_str(), 64);
    strlcpy(config.botName,  webServer.arg("name").c_str(), 32);
    strlcpy(config.location, webServer.arg("location").c_str(), 64);
    strlcpy(config.owmApiKey,webServer.arg("owmkey").c_str(), 48);
    config.setupDone = true;
    saveConfig();
    webServer.send(200, "text/html",
      "<html><body style='background:#030912;color:#00ffe7;font-family:monospace;"
      "display:flex;align-items:center;justify-content:center;height:100vh;font-size:1.2rem'>"
      "✓ Config saved! Chordy is restarting...</body></html>");
    delay(1500);
    ESP.restart();
  } else {
    webServer.send(400, "text/plain", "Bad request");
  }
}

// ──────────────────────────────────────────────────────────────
// JSON Telemetry API
// ──────────────────────────────────────────────────────────────
void handleTelemetry() {
  char json[512];
  snprintf(json, sizeof(json),
    "{"
    "\"temp_c\":%.1f,"
    "\"humidity\":%.0f,"
    "\"ldr\":%d,"
    "\"pir\":%s,"
    "\"weather_temp\":%.1f,"
    "\"weather_wind\":%.1f,"
    "\"weather_desc\":\"%s\","
    "\"weather_valid\":%s,"
    "\"state\":%d,"
    "\"timer_active\":%s,"
    "\"timer_remaining\":%lu"
    "}",
    sensorData.tempC, sensorData.humidity, sensorData.ldrRaw,
    sensorData.pirMotion ? "true" : "false",
    weatherData.tempC, weatherData.windKph,
    weatherData.description,
    weatherData.valid ? "true" : "false",
    (int)currentState,
    timerActive ? "true" : "false",
    timerActive ? (timerEndMs - millis()) / 1000 : 0UL
  );
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "application/json", json);
}

void handleSaveSettings() {
  if (webServer.hasArg("plain")) {
    StaticJsonDocument<256> doc;
    if (!deserializeJson(doc, webServer.arg("plain"))) {
      if (doc.containsKey("botName"))  strlcpy(config.botName,  doc["botName"],  32);
      if (doc.containsKey("location")) strlcpy(config.location, doc["location"], 64);
      if (doc.containsKey("owmKey"))   strlcpy(config.owmApiKey,doc["owmKey"],   48);
      if (doc.containsKey("eyeR"))  config.eyeR = doc["eyeR"];
      if (doc.containsKey("eyeG"))  config.eyeG = doc["eyeG"];
      if (doc.containsKey("eyeB"))  config.eyeB = doc["eyeB"];
      saveConfig();
      webServer.send(200, "application/json", "{\"ok\":true}");
      return;
    }
  }
  webServer.send(400, "application/json", "{\"ok\":false}");
}

void handleTrigger() {
  if (webServer.hasArg("plain")) {
    StaticJsonDocument<64> doc;
    if (!deserializeJson(doc, webServer.arg("plain"))) {
      int animId = doc["anim"] | 0;
      if (animId >= 0 && animId < ANIM_COUNT) {
        forcedAnim = (AnimID)animId;
        webServer.send(200, "application/json", "{\"ok\":true}");
        return;
      }
    }
  }
  webServer.send(400, "application/json", "{\"ok\":false}");
}

void handleSetTimer() {
  if (webServer.hasArg("plain")) {
    StaticJsonDocument<64> doc;
    if (!deserializeJson(doc, webServer.arg("plain"))) {
      int mins = doc["minutes"] | 0;
      if (mins > 0) {
        timerEndMs  = millis() + (unsigned long)mins * 60000UL;
        timerActive = true;
      } else {
        timerActive = false;
      }
      webServer.send(200, "application/json", "{\"ok\":true}");
      return;
    }
  }
  webServer.send(400, "application/json", "{\"ok\":false}");
}

void handleNotFound() {
  if (_apMode) { handleSetup(); return; }
  webServer.send(404, "text/plain", "404 Not Found");
}

// ──────────────────────────────────────────────────────────────
// Full Sci-Fi / Cyberpunk Dashboard
// ──────────────────────────────────────────────────────────────
void handleDashboard() {
  // Served as a single large HTML string
  // The full page is defined in WebDashboard.h to keep this file manageable
  webServer.send(200, "text/html", DASHBOARD_HTML);
}
