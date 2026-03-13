// ============================================================
//  WebServer.ino  —  Chordy v2.0
//  Added: factory reset, buzzer control, eye style, speed,
//         virtual buttons, seconds timer, new anim endpoints
// ============================================================
#include "Config.h"
#include "WebDashboard.h"

extern CConfig       config;
extern SensorData    sensorData;
extern WeatherData   weatherData;
extern RobotState    currentState;
extern AnimID        forcedAnim;
extern bool          timerActive;
extern unsigned long timerEndMs;
extern unsigned long timerTotalMs;
extern unsigned long configuredTimerMs;
extern bool          isAsleep;
extern DisplayScreen currentScreen;
extern void          factoryReset();
extern void          animPlay(AnimID);

static bool _apMode = false;

void webServerInit(bool apMode) {
  _apMode = apMode;
  if (apMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    dnsServer.start(53, "*", WiFi.softAPIP());
  } else {
    WiFi.mode(WIFI_STA);
  }
  // Routes
  webServer.on("/",                    HTTP_GET,  []{ if (_apMode) handleSetupPage(); else handleDashboard(); });
  webServer.on("/setup",               HTTP_GET,  handleSetupPage);
  webServer.on("/save",                HTTP_POST, handleSave);
  webServer.on("/api/telemetry",       HTTP_GET,  handleTelemetry);
  webServer.on("/api/settings",        HTTP_POST, handleSaveSettings);
  webServer.on("/api/trigger",         HTTP_POST, handleTrigger);
  webServer.on("/api/timer",           HTTP_POST, handleSetTimer);
  webServer.on("/api/button",          HTTP_POST, handleVirtualButton);
  webServer.on("/api/factoryreset",    HTTP_POST, handleFactoryReset);
  webServer.on("/api/screen",          HTTP_POST, handleSetScreen);
  webServer.onNotFound([]{ if (_apMode) handleSetupPage(); else webServer.send(404,"text/plain","404"); });
  webServer.begin();
}

void webServerTick() { webServer.handleClient(); }
void dnsServerTick() { dnsServer.processNextRequest(); }

// ── Setup portal ─────────────────────────────────────────────
void handleSetupPage() {
  webServer.send(200, "text/html", R"raw(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Chordy Setup</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@700&family=Share+Tech+Mono&display=swap');
*{box-sizing:border-box;margin:0;padding:0}
body{background:#030912;color:#00ffe7;font-family:'Share Tech Mono',monospace;min-height:100vh;display:flex;align-items:center;justify-content:center}
.card{background:#061525;border:1px solid #00ffe740;border-radius:16px;padding:40px 32px;width:100%;max-width:440px;box-shadow:0 0 40px #00ffe720}
h1{font-family:'Orbitron',sans-serif;font-size:1.8rem;color:#00ffe7;text-shadow:0 0 20px #00ffe780;margin-bottom:6px;text-align:center}
.sub{color:#00ffe780;text-align:center;font-size:.85rem;margin-bottom:28px}
label{display:block;font-size:.75rem;color:#00ffe7aa;margin:14px 0 5px;letter-spacing:2px}
input,select{width:100%;background:#030912;border:1px solid #00ffe740;border-radius:8px;padding:10px 14px;color:#00ffe7;font-family:'Share Tech Mono',monospace;font-size:.9rem;outline:none}
input:focus,select:focus{border-color:#00ffe7}
select option{background:#030912}
button{margin-top:24px;width:100%;background:linear-gradient(90deg,#00ffe720,#00ffe740);border:1px solid #00ffe7;border-radius:8px;padding:13px;color:#00ffe7;font-family:'Orbitron',sans-serif;font-size:.9rem;font-weight:700;letter-spacing:2px;cursor:pointer;text-transform:uppercase}
button:hover{background:linear-gradient(90deg,#00ffe740,#00ffe770);box-shadow:0 0 20px #00ffe740}
</style></head><body>
<div class="card">
<div style="text-align:center;font-size:2rem;margin-bottom:8px">👾</div>
<h1>CHORDY</h1>
<p class="sub">First-time Setup — all data saved to flash</p>
<form action="/save" method="POST">
<label>BOT NAME</label><input name="name" placeholder="Chordy" value="Chordy">
<label>WIFI SSID</label><input name="ssid" placeholder="Your WiFi name">
<label>WIFI PASSWORD</label><input name="pass" type="password" placeholder="••••••••">
<label>LOCATION (for weather)</label><input name="location" placeholder="Dortmund">
<label>EYE STYLE</label>
<select name="eyestyle">
<option value="0">Round (default)</option>
<option value="1">Oval (tall)</option>
<option value="2">Cute (anime)</option>
<option value="3">Wide</option>
</select>
<button type="submit">▶ INITIALIZE CHORDY</button>
</form>
</div></body></html>
)raw");
}

void handleSave() {
  if (webServer.hasArg("ssid")) {
    strlcpy(config.wifiSSID, webServer.arg("ssid").c_str(), 64);
    strlcpy(config.wifiPass, webServer.arg("pass").c_str(), 64);
    strlcpy(config.botName,  webServer.arg("name").c_str(), 32);
    strlcpy(config.location, webServer.arg("location").c_str(), 64);
    config.eyeStyle  = webServer.arg("eyestyle").toInt();
    config.setupDone = true;
    saveConfig();
    webServer.send(200, "text/html",
      "<html><body style='background:#030912;color:#00ffe7;font-family:monospace;"
      "display:flex;align-items:center;justify-content:center;height:100vh;font-size:1.2rem'>"
      "✓ Saved! Chordy restarting...</body></html>");
    delay(1500);
    ESP.restart();
  } else {
    webServer.send(400, "text/plain", "Bad request");
  }
}

// ── Telemetry JSON ────────────────────────────────────────────
void handleTelemetry() {
  struct tm ti; bool hasTime = getLocalTime(&ti);
  char timeBuf[32] = "--:--:--";
  char dateBuf[20] = "--/--/----";
  if (hasTime) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
    snprintf(dateBuf, sizeof(dateBuf), "%02d/%02d/%04d", ti.tm_mday, ti.tm_mon+1, ti.tm_year+1900);
  }

  char json[768];
snprintf(json, sizeof(json),
  "{"
  "\"temp_c\":%.1f,\"humidity\":%.0f,\"ldr\":%d,\"pir\":%s,"
  "\"weather_temp\":%.1f,\"weather_wind\":%.1f,\"weather_desc\":\"%s\","
  "\"weather_code\":%d,\"weather_valid\":%s,"
  "\"state\":%d,\"screen\":%d,"
  "\"timer_active\":%s,\"timer_remaining\":%lu,\"timer_total\":%lu,"
  "\"configured_timer\":%lu,"
  "\"buzzer\":%s,\"buz_vol\":%d,\"anim_speed\":%d,"
  "\"eye_style\":%d,\"bot_name\":\"%s\",\"location\":\"%s\","
  "\"time\":\"%s\",\"date\":\"%s\","
  "\"eye_r\":%d,\"eye_g\":%d,\"eye_b\":%d"
  "}",
  sensorData.tempC, sensorData.humidity, sensorData.ldrRaw,
  sensorData.pirMotion ? "true":"false",
  weatherData.tempC, weatherData.windKph, weatherData.description,
  weatherData.conditionCode, weatherData.valid ? "true":"false",
  (int)currentState, (int)currentScreen,
  timerActive ? "true":"false",
  timerActive ? (timerEndMs - millis()) / 1000 : 0UL,
  timerTotalMs / 1000,
  configuredTimerMs / 1000,
  config.buzzerEnabled ? "true":"false",
  config.buzzerVolume, config.animSpeed,
  config.eyeStyle, config.botName, config.location,
  timeBuf, dateBuf,
  config.eyeR, config.eyeG, config.eyeB
);
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "application/json", json);
}

// ── Save settings ─────────────────────────────────────────────
void handleSaveSettings() {
  if (!webServer.hasArg("plain")) { webServer.send(400,"application/json","{\"ok\":false}"); return; }
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, webServer.arg("plain"))) { webServer.send(400,"application/json","{\"ok\":false}"); return; }

  if (doc.containsKey("botName"))    strlcpy(config.botName,  doc["botName"],  32);
  if (doc.containsKey("location"))   strlcpy(config.location, doc["location"], 64);
  if (doc.containsKey("eyeR"))       config.eyeR          = doc["eyeR"];
  if (doc.containsKey("eyeG"))       config.eyeG          = doc["eyeG"];
  if (doc.containsKey("eyeB"))       config.eyeB          = doc["eyeB"];
  if (doc.containsKey("eyeStyle"))   config.eyeStyle      = doc["eyeStyle"];
  if (doc.containsKey("animSpeed"))  config.animSpeed      = doc["animSpeed"];
  if (doc.containsKey("buzzer"))     config.buzzerEnabled  = doc["buzzer"];
  if (doc.containsKey("buzVol"))     config.buzzerVolume   = doc["buzVol"];
  saveConfig();
  webServer.send(200, "application/json", "{\"ok\":true}");
}

// ── Trigger animation ─────────────────────────────────────────
void handleTrigger() {
  if (!webServer.hasArg("plain")) { webServer.send(400,"application/json","{\"ok\":false}"); return; }
  StaticJsonDocument<64> doc;
  if (!deserializeJson(doc, webServer.arg("plain"))) {
    int animId = doc["anim"] | 0;
    if (animId >= 0 && animId < ANIM_COUNT) {
      forcedAnim = (AnimID)animId;
      webServer.send(200, "application/json", "{\"ok\":true}");
      return;
    }
  }
  webServer.send(400, "application/json", "{\"ok\":false}");
}

// ── Set timer (with seconds support) ─────────────────────────
void handleSetTimer() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, webServer.arg("plain"))) {
    webServer.send(400, "application/json", "{\"ok\":false}");
    return;
  }

  int mins = doc["minutes"] | 0;
  int secs = doc["seconds"] | 0;

  if (mins < 0) mins = 0;
  if (secs < 0) secs = 0;
  if (secs > 59) secs = 59;

  configuredTimerMs = (unsigned long)mins * 60000UL + (unsigned long)secs * 1000UL;

  if (configuredTimerMs > 0) {
    timerTotalMs   = configuredTimerMs;
    timerEndMs     = millis() + timerTotalMs;
    timerActive    = true;
    currentState   = STATE_TIMER_RUNNING;
    currentScreen  = SCREEN_FACE;
    animPlay(ANIM_THINKING);
    if (config.buzzerEnabled) buzzerMelody(2);
  } else {
    timerActive    = false;
    timerTotalMs   = 0;
    timerEndMs     = 0;
    currentState   = STATE_IDLE;
    currentScreen  = SCREEN_FACE;
    animPlay(ANIM_IDLE);
    if (config.buzzerEnabled) buzzerTone(400, 200);
  }

  webServer.send(200, "application/json", "{\"ok\":true}");
}

// ── Virtual button press from web ─────────────────────────────
void handleVirtualButton() {
  if (!webServer.hasArg("plain")) { webServer.send(400,"application/json","{\"ok\":false}"); return; }
  StaticJsonDocument<64> doc;
  if (!deserializeJson(doc, webServer.arg("plain"))) {
    int btn = doc["btn"] | -1;  // 0=POWER 1=SELECT 2=INTERACT 3=EXTRA
    switch (btn) {
      case 0: // power toggle
        isAsleep = !isAsleep;
        if (isAsleep) { currentState = STATE_SLEEPING; animPlay(ANIM_SLEEPY); if(config.buzzerEnabled) buzzerMelody(3); }
        else          { currentState = STATE_IDLE;     animPlay(ANIM_IDLE);   if(config.buzzerEnabled) buzzerMelody(0); }
        break;
      case 1: // screen cycle
        currentScreen = (DisplayScreen)((currentScreen + 1) % SCREEN_COUNT);
        if (config.buzzerEnabled) buzzerTone(1000, 60);
        break;
      case 2: // pet
        currentState = STATE_VERY_HAPPY;
        animPlay(ANIM_HEART_EYES);
        if (config.buzzerEnabled) buzzerMelody(1);
        break;
      case 3: // timer toggle
        if (timerActive) {
          timerActive = false;
          if (config.buzzerEnabled) buzzerTone(400, 200);
        } else {
          if (configuredTimerMs == 0) configuredTimerMs = 25UL * 60UL * 1000UL;
              timerTotalMs = configuredTimerMs;
              timerEndMs   = millis() + timerTotalMs;
              timerActive  = true;
              currentState = STATE_TIMER_RUNNING;
              currentScreen = SCREEN_FACE;
              animPlay(ANIM_THINKING);
              if (config.buzzerEnabled) buzzerMelody(2);
        }
        break;
    }
    webServer.send(200, "application/json", "{\"ok\":true}");
    return;
  }
  webServer.send(400, "application/json", "{\"ok\":false}");
}

// ── Factory reset ─────────────────────────────────────────────
void handleFactoryReset() {
  webServer.send(200, "application/json", "{\"ok\":true,\"msg\":\"Resetting...\"}");
  delay(300);
  factoryReset();
}

// ── Set screen ────────────────────────────────────────────────
void handleSetScreen() {
  if (!webServer.hasArg("plain")) { webServer.send(400,"application/json","{\"ok\":false}"); return; }
  StaticJsonDocument<64> doc;
  if (!deserializeJson(doc, webServer.arg("plain"))) {
    int s = doc["screen"] | 0;
    if (s >= 0 && s < SCREEN_COUNT) {
      currentScreen = (DisplayScreen)s;
      webServer.send(200, "application/json", "{\"ok\":true}");
      return;
    }
  }
  webServer.send(400, "application/json", "{\"ok\":false}");
}

// ── Dashboard ─────────────────────────────────────────────────
void handleDashboard() {
  webServer.send_P(200, "text/html", DASHBOARD_HTML);
}
