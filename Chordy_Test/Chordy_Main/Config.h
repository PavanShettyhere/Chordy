// ============================================================
//  Config.h  —  Chordy Desk Companion Robot
// ============================================================
#pragma once

#include <Arduino.h>

// ── OLED (I2C SSD1306) ──────────────────────────────────────
#define OLED_SDA        21
#define OLED_SCL        22
#define OLED_ADDR       0x3C
#define OLED_WIDTH      128
#define OLED_HEIGHT     64

// ── Sensors ─────────────────────────────────────────────────
#define DHT_PIN         17
#define DHT_TYPE        DHT11

#define LDR_PIN         34
#define LDR_DARK_THRESH 800
#define LDR_BRIGHT_THRESH 200
#define PIR_PIN         19

// ── Output ──────────────────────────────────────────────────
#define BUZZER_PIN      18

// ── Buttons (Active LOW + internal pullup) ──────────────────
#define BTN_POWER       4
#define BTN_SELECT      5
#define BTN_INTERACT    13
#define BTN_EXTRA       14
#define BTN_DEBOUNCE_MS 45

// ── Timing constants ────────────────────────────────────────
#define PIR_LONG_ABSENCE_MS      7200000UL
#define WEATHER_FETCH_INTERVAL_MS 600000UL
#define SENSOR_READ_INTERVAL_MS    5000UL
#define LDR_SAMPLE_INTERVAL_MS      200UL
#define LDR_SUDDEN_CHANGE_DELTA      600

// ── WiFi / Server ───────────────────────────────────────────
#define AP_SSID         "Chordy_Setup"
#define AP_PASS         ""
#define WEBSERVER_PORT  80

// ── Preferences namespace ───────────────────────────────────
#define PREF_NAMESPACE  "chordy"

// ── Weather API ─────────────────────────────────────────────
#define WEATHER_API_HOST      "api.open-meteo.com"
#define WEATHER_API_PATH      "/v1/forecast"
#define WEATHER_GEOCODE_HOST  "geocoding-api.open-meteo.com"
#define WEATHER_GEOCODE_PATH  "/v1/search"
#define TEMP_HOT_C        28.0f
#define TEMP_COLD_C       15.0f

struct CConfig {
  char  botName[32]     = "Chordy";
  char  wifiSSID[64]    = "";
  char  wifiPass[64]    = "";
  char  location[64]    = "Dortmund";
  char  owmApiKey[48]   = "";
  uint8_t eyeR          = 0;
  uint8_t eyeG          = 255;
  uint8_t eyeB          = 200;
  uint8_t eyeStyle      = 0;     // 0 rounded, 1 sharp, 2 sleepy
  uint8_t buzzerVolume  = 70;    // 0..100
  uint8_t animSpeedPct  = 100;   // 40..180
  bool    buzzerEnabled = true;
  bool    setupDone     = false;
};

struct SensorData {
  float   tempC         = 0.0f;
  float   humidity      = 0.0f;
  int     ldrRaw        = 0;
  bool    pirMotion     = false;
  unsigned long lastMotionMs = 0;
};

struct WeatherData {
  float   tempC         = 0.0f;
  float   windKph       = 0.0f;
  int     conditionCode = 800;
  char    description[48] = "clear sky";
  bool    valid         = false;
};

enum RobotState {
  STATE_BOOT,
  STATE_WIFI_SETUP,
  STATE_CONNECTING,
  STATE_IDLE,
  STATE_HAPPY,
  STATE_VERY_HAPPY,
  STATE_SCARED,
  STATE_SLEEPY,
  STATE_BLINKING_BRIGHT,
  STATE_WEATHER_COLD,
  STATE_WEATHER_HOT,
  STATE_WEATHER_RAINY,
  STATE_INTERACTING,
  STATE_TIMER_RUNNING,
  STATE_SLEEPING
};

enum AnimID {
  ANIM_IDLE = 0,
  ANIM_BLINK,
  ANIM_HAPPY,
  ANIM_VERY_HAPPY,
  ANIM_SCARED,
  ANIM_SLEEPY,
  ANIM_SQUINT,
  ANIM_HEART_EYES,
  ANIM_LOOK_LEFT,
  ANIM_LOOK_RIGHT,
  ANIM_WEATHER_COLD,
  ANIM_WEATHER_HOT,
  ANIM_WEATHER_RAIN,
  ANIM_STARTUP,
  ANIM_COUNT
};

enum UIScreen {
  SCREEN_FACE = 0,
  SCREEN_MENU,
  SCREEN_SETTINGS,
  SCREEN_COUNT
};
