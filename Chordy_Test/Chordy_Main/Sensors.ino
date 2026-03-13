// ============================================================
//  Sensors.ino  —  DHT11 / LDR / PIR sensor management
// ============================================================

#include "Config.h"

extern SensorData sensorData;

// ──────────────────────────────────────────────────────────────
void sensorsInit() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);  // ADC on GPIO34
  Serial.println("[Sensors] Initialized DHT11, LDR, PIR");
}

// ──────────────────────────────────────────────────────────────
void sensorsRead() {
  // --- DHT11 ---
  float t = dht11.readTemperature();
  float h = dht11.readHumidity();
  if (!isnan(t)) sensorData.tempC    = t;
  if (!isnan(h)) sensorData.humidity = h;

  // --- LDR (12-bit ADC, 0–4095) ---
  sensorData.ldrRaw = analogRead(LDR_PIN);

  // --- PIR ---
  bool motion = digitalRead(PIR_PIN) == HIGH;
  if (motion && !sensorData.pirMotion) {
    // Rising edge — update last motion time
    sensorData.lastMotionMs = millis();
  }
  sensorData.pirMotion = motion;

  // Debug
  Serial.printf("[Sensors] T=%.1fC H=%.0f%% LDR=%d PIR=%d\n",
                sensorData.tempC, sensorData.humidity,
                sensorData.ldrRaw, sensorData.pirMotion ? 1 : 0);
}
