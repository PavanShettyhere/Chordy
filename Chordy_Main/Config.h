// ============================================================
//  Config.h  —  Chordy Desk Companion Robot  v2.0
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

#define LDR_PIN         34
#define LDR_DARK_THRESH      800
#define LDR_BRIGHT_THRESH    200
#define LDR_SUDDEN_CHANGE_DELTA 600
#define LDR_REACTION_DURATION_MS 4000UL   // 4 seconds for LDR reactions

#define PIR_PIN         19

// ── Output ──────────────────────────────────────────────────
#define BUZZER_PIN      18

// ── Buttons (Active LOW + internal pullup) ──────────────────
#define BTN_POWER       4    // Power / Sleep toggle
#define BTN_SELECT      5    // Screen cycle
#define BTN_INTERACT    13   // Pet Chordy
#define BTN_EXTRA       14   // Timer / Extra

// ── Debounce ────────────────────────────────────────────────
#define BTN_DEBOUNCE_MS 50

// ── Timing constants ─────────────────────────────────────────
#define PIR_LONG_ABSENCE_MS       7200000UL
#define WEATHER_FETCH_INTERVAL_MS 600000UL
#define SENSOR_READ_INTERVAL_MS   5000UL
#define LDR_SAMPLE_INTERVAL_MS    300UL
#define IDLE_ANIM_MIN_MS          4000UL
#define IDLE_ANIM_MAX_EXTRA_MS    8000UL

// ── WiFi / Server ────────────────────────────────────────────
#define AP_SSID         "Chordy_Setup"
#define AP_PASS         ""
#define WEBSERVER_PORT  80

// ── Preferences namespace ────────────────────────────────────
#define PREF_NAMESPACE  "chordy"

// ── Weather / Time API ───────────────────────────────────────
#define WEATHER_GEOCODE_HOST  "geocoding-api.open-meteo.com"
#define WEATHER_GEOCODE_PATH  "/v1/search"
#define WEATHER_API_HOST      "api.open-meteo.com"
#define WEATHER_API_PATH      "/v1/forecast"
#define TIME_API_HOST         "worldtimeapi.org"
#define TEMP_HOT_C        28.0f
#define TEMP_COLD_C       15.0f

// ──────────────────────────────────────────────────────────────
//  Eye Styles
// ──────────────────────────────────────────────────────────────
enum EyeStyle {
  EYE_ROUND = 0,    // default rounded rect
  EYE_OVAL,         // taller oval
  EYE_CUTE,         // smaller, rounder (more anime)
  EYE_WIDE,         // extra wide
  EYE_STYLE_COUNT
};

// ──────────────────────────────────────────────────────────────
//  Display screens (BTN_SELECT cycles through)
// ──────────────────────────────────────────────────────────────
enum DisplayScreen {
  SCREEN_FACE = 0,
  SCREEN_CLOCK,
  SCREEN_WEATHER,
  SCREEN_SENSORS,
  SCREEN_COUNT
};

// ──────────────────────────────────────────────────────────────
//  Data Structures
// ──────────────────────────────────────────────────────────────
struct CConfig {
  char     botName[32]    = "Chordy";
  char     wifiSSID[64]   = "";
  char     wifiPass[64]   = "";
  char     location[64]   = "Dortmund";
  uint8_t  eyeR           = 0;
  uint8_t  eyeG           = 255;
  uint8_t  eyeB           = 200;
  uint8_t  eyeStyle       = EYE_ROUND;
  uint8_t  animSpeed      = 5;    // 1=slow..10=fast
  bool     buzzerEnabled  = true;
  uint8_t  buzzerVolume   = 7;    // 1-10
  bool     setupDone      = false;
};

struct SensorData {
  float         tempC         = 0.0f;
  float         humidity      = 0.0f;
  int           ldrRaw        = 0;
  bool          pirMotion     = false;
  unsigned long lastMotionMs  = 0;
};

struct WeatherData {
  float   tempC           = 0.0f;
  float   windKph         = 0.0f;
  int     conditionCode   = 0;
  char    description[48] = "unknown";
  bool    valid           = false;
};

struct TimeData {
  int  hour    = 0;
  int  minute  = 0;
  int  second  = 0;
  int  day     = 0;
  int  month   = 0;
  int  year    = 2025;
  bool valid   = false;
  unsigned long fetchedAtMs = 0;
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
  STATE_SLEEPING,
  STATE_CLOCK,
  STATE_WEATHER_INFO,
  STATE_SENSORS_INFO
};

// ──────────────────────────────────────────────────────────────
//  Animation IDs
// ──────────────────────────────────────────────────────────────
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
  // New v2 emotions
  ANIM_PLAYING_GUITAR,
  ANIM_WHISTLING,
  ANIM_FLY,
  ANIM_TYPING,
  ANIM_COFFEE,
  ANIM_MAGNIFY,
  ANIM_THINKING,
  ANIM_LOOK_UP,
  ANIM_LOOK_DOWN,
  ANIM_WINK,
  ANIM_COUNT
};
