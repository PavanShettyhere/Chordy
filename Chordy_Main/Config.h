// ============================================================
//  Config.h  —  Chordy Desk Companion Robot
//  All pin definitions, structs, and compile-time constants
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

#define LDR_PIN         34   // Analog input (ADC1_CH6)
#define LDR_DARK_THRESH 800  // ADC value above which = dark (0-4095)
#define LDR_BRIGHT_THRESH 200 // ADC value below which = very bright

#define PIR_PIN         19   // PIR motion sensor (HIGH = motion)

// ── Output ──────────────────────────────────────────────────
#define BUZZER_PIN      18   // PWM-capable pin

// ── Buttons (Active LOW + internal pullup) ──────────────────
#define BTN_POWER       4    // Power / Sleep toggle
#define BTN_SELECT      5    // Menu / Select
#define BTN_INTERACT    13   // Human interaction / pet button
#define BTN_EXTRA       14   // Extra / Timer

// ── Debounce ────────────────────────────────────────────────
#define BTN_DEBOUNCE_MS 50

// ── Timing constants (milliseconds) ─────────────────────────
#define PIR_LONG_ABSENCE_MS   7200000UL   // 2 hours → very happy greeting
#define WEATHER_FETCH_INTERVAL_MS 600000UL // 10 minutes
#define SENSOR_READ_INTERVAL_MS   5000UL
#define LDR_SAMPLE_INTERVAL_MS    200UL
#define LDR_SUDDEN_CHANGE_DELTA   600     // Raw ADC units

// ── WiFi / Server ────────────────────────────────────────────
#define AP_SSID         "Chordy_Setup"
#define AP_PASS         ""              // Open AP
#define WEBSERVER_PORT  80

// ── Preferences namespace ────────────────────────────────────
#define PREF_NAMESPACE  "chordy"

// ── Weather API ──────────────────────────────────────────────
#define WEATHER_API_HOST  "api.openweathermap.org"
#define WEATHER_API_PATH  "/data/2.5/weather"
#define TEMP_HOT_C        28.0f
#define TEMP_COLD_C       15.0f

// ──────────────────────────────────────────────────────────────
//  Data Structures
// ──────────────────────────────────────────────────────────────

struct CConfig {
  char  botName[32]     = "Chordy";
  char  wifiSSID[64]    = "";
  char  wifiPass[64]    = "";
  char  location[64]    = "London";
  char  owmApiKey[48]   = "";
  uint8_t eyeR          = 0;
  uint8_t eyeG          = 255;
  uint8_t eyeB          = 200;
  bool  setupDone       = false;
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
  int     conditionCode = 800;  // 800 = clear sky
  char    description[48] = "clear sky";
  bool    valid         = false;
};

// ──────────────────────────────────────────────────────────────
//  Robot State Machine
// ──────────────────────────────────────────────────────────────
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

// Animation IDs for developer panel triggers
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
