// ============================================================
//  Animations.ino  —  Eye renderer + simple UI screens + buzzer
// ============================================================

#include "Config.h"

extern Adafruit_SSD1306 display;
extern CConfig config;
extern WeatherData weatherData;
extern SensorData sensorData;
extern UIScreen currentScreen;
extern bool isAsleep;

#define EYE_L_CX 36
#define EYE_R_CX 92
#define EYE_CY   28
#define EYE_W    26
#define EYE_H    22
#define PUPIL_R   5

static AnimID animCurrent = ANIM_IDLE;
static int animFrame = 0;
static unsigned long animLastMs = 0;
static int animFrameDelay = 120;
static int eyeOffX = 0;
static int eyeOffY = 0;
static int blinkH = EYE_H;

static uint16_t buzzerDuty() {
  uint8_t volume = constrain(config.buzzerVolume, 0, 100);
  return map(volume, 0, 100, 0, 768);
}

static int scaledDelay(int baseMs) {
  int pct = constrain((int)config.animSpeedPct, 40, 180);
  long scaled = ((long)baseMs * 100L) / pct;
  if (scaled < 25) scaled = 25;
  if (scaled > 450) scaled = 450;
  return (int)scaled;
}

void drawSharpEye(int cx, int cy, int w, int h) {
  display.fillTriangle(cx - w / 2, cy, cx, cy - h / 2, cx + w / 2, cy, SSD1306_WHITE);
  display.fillTriangle(cx - w / 2, cy, cx, cy + h / 2, cx + w / 2, cy, SSD1306_WHITE);
}

void drawEyeShell(int cx, int cy, int w, int h) {
  int style = config.eyeStyle % 3;
  if (style == 1) {
    drawSharpEye(cx, cy, w, h);
  } else if (style == 2) {
    display.fillRoundRect(cx - w / 2, cy - h / 2, w, h, 8, SSD1306_WHITE);
    display.fillRect(cx - w / 2, cy + h / 4, w, h / 4, SSD1306_BLACK);
  } else {
    display.fillRoundRect(cx - w / 2, cy - h / 2, w, h, h / 3, SSD1306_WHITE);
  }
}

void drawEye(int cx, int cy, int w, int h, int pox, int poy, int pupilSize = PUPIL_R) {
  drawEyeShell(cx, cy, w, h);
  display.fillCircle(cx + pox, cy + poy, pupilSize, SSD1306_BLACK);
  display.fillCircle(cx + pox + 2, cy + poy - 2, 1, SSD1306_WHITE);
}

void drawEyelid(int cx, int cy, int w, int h, int lidH, bool top = true) {
  if (lidH <= 0) return;
  int y = top ? (cy - h / 2) : (cy + h / 2 - lidH);
  display.fillRect(cx - w / 2 - 1, y, w + 2, lidH, SSD1306_BLACK);
}

void drawEyes(int pox, int poy, int lidT = 0, int lidB = 0, int leftExtra = 0, int rightExtra = 0) {
  int lh = max(6, blinkH + leftExtra);
  int rh = max(6, blinkH + rightExtra);
  drawEye(EYE_L_CX, EYE_CY, EYE_W, lh, pox, poy);
  drawEye(EYE_R_CX, EYE_CY, EYE_W, rh, pox, poy);
  if (lidT > 0) {
    drawEyelid(EYE_L_CX, EYE_CY, EYE_W, lh, lidT, true);
    drawEyelid(EYE_R_CX, EYE_CY, EYE_W, rh, lidT, true);
  }
  if (lidB > 0) {
    drawEyelid(EYE_L_CX, EYE_CY, EYE_W, lh, lidB, false);
    drawEyelid(EYE_R_CX, EYE_CY, EYE_W, rh, lidB, false);
  }
}

void drawHeart(int cx, int cy, int size) {
  display.fillCircle(cx - size / 3, cy - size / 4, size / 3 + 1, SSD1306_WHITE);
  display.fillCircle(cx + size / 3, cy - size / 4, size / 3 + 1, SSD1306_WHITE);
  for (int y = 0; y < size + 2; ++y) {
    int half = max(0, size - y);
    display.drawFastHLine(cx - half, cy + y / 2, half * 2 + 1, SSD1306_WHITE);
  }
}

void drawCheeks() {
  display.drawCircle(18, 42, 4, SSD1306_WHITE);
  display.drawCircle(110, 42, 4, SSD1306_WHITE);
}

void drawArms(int frame) {
  int lift = (frame / 4) % 2 ? 2 : -2;
  display.drawLine(22, 46, 12, 54 + lift, SSD1306_WHITE);
  display.drawLine(106, 46, 116, 54 - lift, SSD1306_WHITE);
  display.drawCircle(12, 54 + lift, 2, SSD1306_WHITE);
  display.drawCircle(116, 54 - lift, 2, SSD1306_WHITE);
}

void drawBlanket() {
  for (int x = 0; x < 128; x += 8) {
    display.fillRect(x, 50, 8, 14, (x / 8 % 2) ? SSD1306_WHITE : SSD1306_BLACK);
  }
  display.drawRect(0, 50, 128, 14, SSD1306_WHITE);
}

void drawFan(int frame) {
  int cx = 64, cy = 54, r = 8;
  float angle = frame * 0.28f;
  for (int b = 0; b < 3; ++b) {
    float a = angle + b * 2.094f;
    int x1 = cx + (int)(cos(a) * r);
    int y1 = cy + (int)(sin(a) * r);
    int x2 = cx + (int)(cos(a + 1.0f) * r);
    int y2 = cy + (int)(sin(a + 1.0f) * r);
    display.fillTriangle(cx, cy, x1, y1, x2, y2, SSD1306_WHITE);
  }
}

void drawRain(int frame) {
  for (int i = 0; i < 7; ++i) {
    int rx = (i * 17 + frame * 3) % 120 + 4;
    int ry = (frame * 4 + i * 11) % 40 + 2;
    display.drawLine(rx, ry, rx - 2, ry + 5, SSD1306_WHITE);
  }
  display.drawCircle(32, 7, 6, SSD1306_WHITE);
  display.drawCircle(40, 5, 7, SSD1306_WHITE);
  display.drawCircle(50, 7, 6, SSD1306_WHITE);
  display.drawFastHLine(26, 10, 30, SSD1306_WHITE);
}

void drawZzz(int frame) {
  int offset = (frame * 2) % 14;
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(95, 8 + offset / 2); display.print("z");
  display.setCursor(104, 4 + offset / 3); display.print("Z");
  display.setCursor(113, 1 + offset / 4); display.print("Z");
}

void drawNameTag() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int len = strlen(config.botName) * 6;
  display.setCursor((OLED_WIDTH - len) / 2, 56);
  display.print(config.botName);
}

void drawMenuScreen() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); display.print("MENU");
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  display.setCursor(0, 16); display.print("Select: next screen");
  display.setCursor(0, 28); display.print("Interact: happy");
  display.setCursor(0, 40); display.print("Extra: 25m timer");
  display.setCursor(0, 54); display.print(isAsleep ? "Status: sleeping" : "Status: awake");
}

void drawSettingsScreen() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); display.print("SETTINGS");
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  display.setCursor(0, 16); display.printf("Eye style: %d", config.eyeStyle + 1);
  display.setCursor(0, 28); display.printf("Anim spd: %u%%", config.animSpeedPct);
  display.setCursor(0, 40); display.printf("Buzzer: %s", config.buzzerEnabled ? "on" : "off");
  display.setCursor(0, 52); display.printf("Vol: %u%%", config.buzzerVolume);
}

void animInit() {
  display.clearDisplay();
  display.display();
  blinkH = EYE_H;
  animFrame = 0;
}

void animPlay(AnimID id) {
  animCurrent = id;
  animFrame = 0;
  blinkH = EYE_H;
  eyeOffX = 0;
  eyeOffY = 0;

  switch (id) {
    case ANIM_IDLE:         animFrameDelay = scaledDelay(140); break;
    case ANIM_BLINK:        animFrameDelay = scaledDelay(65);  break;
    case ANIM_HAPPY:        animFrameDelay = scaledDelay(100); break;
    case ANIM_VERY_HAPPY:   animFrameDelay = scaledDelay(130); break;
    case ANIM_SCARED:       animFrameDelay = scaledDelay(70);  break;
    case ANIM_SLEEPY:       animFrameDelay = scaledDelay(170); break;
    case ANIM_SQUINT:       animFrameDelay = scaledDelay(70);  break;
    case ANIM_HEART_EYES:   animFrameDelay = scaledDelay(130); break;
    case ANIM_LOOK_LEFT:    animFrameDelay = scaledDelay(90);  break;
    case ANIM_LOOK_RIGHT:   animFrameDelay = scaledDelay(90);  break;
    case ANIM_WEATHER_COLD: animFrameDelay = scaledDelay(160); break;
    case ANIM_WEATHER_HOT:  animFrameDelay = scaledDelay(95);  break;
    case ANIM_WEATHER_RAIN: animFrameDelay = scaledDelay(150); break;
    case ANIM_STARTUP:      animFrameDelay = scaledDelay(80);  break;
    default:                animFrameDelay = scaledDelay(120); break;
  }
}

void animTick() {
  unsigned long now = millis();
  if (now - animLastMs < (unsigned long)animFrameDelay) return;
  animLastMs = now;

  display.clearDisplay();

  if (currentScreen == SCREEN_MENU) {
    drawMenuScreen();
    display.display();
    return;
  }
  if (currentScreen == SCREEN_SETTINGS) {
    drawSettingsScreen();
    display.display();
    return;
  }

  switch (animCurrent) {
    case ANIM_IDLE: {
      static const int lookX[] = {0, 0, 1, 2, 1, 0, -1, -2, -1, 0};
      static const int lookY[] = {0, 1, 0, 0, -1, 0, 1, 0, 0, -1};
      int idx = (animFrame / 5) % 10;
      int leftExtra  = (animFrame / 9) % 8 == 3 ? 3 : 0;
      int rightExtra = (animFrame / 11) % 8 == 5 ? -3 : 0;
      if (animFrame % 40 >= 34 && animFrame % 40 <= 36) blinkH = 8; else blinkH = EYE_H;
      drawEyes(lookX[idx], lookY[idx], 0, 0, leftExtra, rightExtra);
      if ((animFrame / 12) % 2 == 0) drawCheeks();
      drawNameTag();
      break;
    }

    case ANIM_BLINK: {
      int seq[] = {EYE_H, 16, 10, 4, 2, 4, 10, 16, EYE_H};
      int n = sizeof(seq) / sizeof(seq[0]);
      if (animFrame >= n) { animPlay(ANIM_IDLE); break; }
      blinkH = seq[animFrame];
      drawEyes(0, 0);
      drawNameTag();
      break;
    }

    case ANIM_HAPPY: {
      int bounce = (animFrame % 12 < 6) ? -1 : 2;
      drawEyes(0, bounce, 0, 0);
      drawArms(animFrame);
      display.drawCircle(64, 56, 6, SSD1306_WHITE);
      display.fillRect(58, 50, 12, 8, SSD1306_BLACK);
      drawNameTag();
      break;
    }

    case ANIM_VERY_HAPPY:
    case ANIM_HEART_EYES: {
      int bounce = (animFrame % 14 < 7) ? -1 : 1;
      drawEyeShell(EYE_L_CX, EYE_CY + bounce, EYE_W, EYE_H);
      drawEyeShell(EYE_R_CX, EYE_CY + bounce, EYE_W, EYE_H);
      drawHeart(EYE_L_CX, EYE_CY - 2 + bounce, 7);
      drawHeart(EYE_R_CX, EYE_CY - 2 + bounce, 7);
      display.fillCircle(EYE_L_CX, EYE_CY + 4 + bounce, 2, SSD1306_BLACK);
      display.fillCircle(EYE_R_CX, EYE_CY + 4 + bounce, 2, SSD1306_BLACK);
      drawArms(animFrame);
      drawCheeks();
      drawNameTag();
      if (animFrame > 70 && animCurrent == ANIM_VERY_HAPPY) animPlay(ANIM_HAPPY);
      break;
    }

    case ANIM_SCARED: {
      int shakeX = (animFrame % 4 < 2) ? -2 : 2;
      blinkH = EYE_H + 4;
      drawEyes(shakeX, 0);
      display.fillCircle(EYE_R_CX + 16, EYE_CY + 8, 2, SSD1306_WHITE);
      display.drawLine(EYE_R_CX + 16, EYE_CY + 4, EYE_R_CX + 16, EYE_CY + 7, SSD1306_WHITE);
      drawNameTag();
      if (animFrame > 60) animPlay(ANIM_IDLE);
      break;
    }

    case ANIM_SLEEPY: {
      blinkH = EYE_H;
      int lidAmount = EYE_H / 2 + (int)(sin(animFrame * 0.1f) * 2);
      drawEyes(0, 2, lidAmount);
      drawZzz(animFrame);
      drawNameTag();
      break;
    }

    case ANIM_SQUINT: {
      int squintSeq[] = {EYE_H, EYE_H / 2, EYE_H / 3, EYE_H / 4, EYE_H / 3, EYE_H / 2, EYE_H};
      int n = sizeof(squintSeq) / sizeof(squintSeq[0]);
      if (animFrame >= n) { animPlay(ANIM_IDLE); break; }
      blinkH = squintSeq[animFrame];
      drawEyes(0, 0, blinkH > 4 ? (EYE_H - blinkH) / 2 : 0);
      drawNameTag();
      break;
    }

    case ANIM_LOOK_LEFT: {
      int pSeq[] = {0, -3, -5, -5, -4, -2, 0};
      int n = sizeof(pSeq) / sizeof(pSeq[0]);
      if (animFrame >= n) { animPlay(ANIM_IDLE); break; }
      drawEyes(pSeq[animFrame], 0);
      drawNameTag();
      break;
    }

    case ANIM_LOOK_RIGHT: {
      int pSeq[] = {0, 3, 5, 5, 4, 2, 0};
      int n = sizeof(pSeq) / sizeof(pSeq[0]);
      if (animFrame >= n) { animPlay(ANIM_IDLE); break; }
      drawEyes(pSeq[animFrame], 0);
      drawNameTag();
      break;
    }

    case ANIM_WEATHER_COLD: {
      int shiverX = (animFrame % 6 < 3) ? -1 : 1;
      drawEyes(shiverX, 0);
      drawBlanket();
      display.setCursor(2, 2); display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
      display.printf("%.0fC Brr", weatherData.tempC);
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    case ANIM_WEATHER_HOT: {
      drawEyes(0, 1);
      drawFan(animFrame);
      display.setCursor(2, 2); display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
      display.printf("%.0fC Hot", weatherData.tempC);
      display.fillCircle(EYE_L_CX - 12, EYE_CY + 10 + (animFrame % 6), 2, SSD1306_WHITE);
      display.fillCircle(EYE_R_CX + 12, EYE_CY + 10 + (animFrame % 5), 2, SSD1306_WHITE);
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    case ANIM_WEATHER_RAIN: {
      drawEyes(0, 0);
      drawRain(animFrame);
      display.setCursor(2, 52); display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
      display.print("Rainy");
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    case ANIM_STARTUP: {
      int growSeq[] = {2, 6, 10, 14, 18, EYE_H, EYE_H};
      int n = sizeof(growSeq) / sizeof(growSeq[0]);
      if (animFrame >= n) {
        drawEyes(0, 0);
        display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
        display.setCursor(30, 54); display.print("Hi! I'm Chordy");
        animPlay(ANIM_IDLE);
      } else {
        blinkH = growSeq[animFrame];
        drawEyes(0, 0);
      }
      break;
    }

    default:
      drawEyes(0, 0);
      drawNameTag();
      break;
  }

  animFrame++;
  display.display();
}

void buzzerTone(int freq, int durationMs) {
  if (!config.buzzerEnabled || config.buzzerVolume == 0 || durationMs <= 0) return;

  ledcAttach(BUZZER_PIN, max(freq, 1), 10);
  ledcWriteTone(BUZZER_PIN, freq);
  ledcWrite(BUZZER_PIN, buzzerDuty());

  unsigned long start = millis();
  while (millis() - start < (unsigned long)durationMs) {
    webServerTick();
    yield();
  }

  ledcWriteTone(BUZZER_PIN, 0);
  ledcDetach(BUZZER_PIN);
}

void buzzerMelody(int id) {
  if (!config.buzzerEnabled || config.buzzerVolume == 0) return;

  struct Note { int freq; int dur; };
  const Note startup[] = {{523,90},{659,90},{784,120},{1047,180}};
  const Note happy[]   = {{784,70},{988,70},{1175,120}};
  const Note timerSet[]= {{880,60},{1047,110}};
  const Note sleep[]   = {{523,180},{440,200},{349,240}};
  const Note alarm[]   = {{1047,90},{0,40},{1047,90},{0,40},{1047,220}};
  const Note scared[]  = {{240,60},{200,60},{170,120}};
  const Note hot[]     = {{659,60},{784,60},{880,80}};
  const Note cold[]    = {{440,120},{392,120},{349,150}};
  const Note rain[]    = {{523,70},{466,70},{392,120}};

  const Note* notes = nullptr;
  int len = 0;
  switch (id) {
    case 0: notes = startup; len = 4; break;
    case 1: notes = happy;   len = 3; break;
    case 2: notes = timerSet;len = 2; break;
    case 3: notes = sleep;   len = 3; break;
    case 4: notes = alarm;   len = 5; break;
    case 5: notes = scared;  len = 3; break;
    case 6: notes = hot;     len = 3; break;
    case 7: notes = cold;    len = 3; break;
    case 8: notes = rain;    len = 3; break;
    default: return;
  }

  for (int i = 0; i < len; ++i) {
    if (notes[i].freq > 0) {
      ledcAttach(BUZZER_PIN, notes[i].freq, 10);
      ledcWriteTone(BUZZER_PIN, notes[i].freq);
      ledcWrite(BUZZER_PIN, buzzerDuty());
    } else {
      ledcWriteTone(BUZZER_PIN, 0);
    }

    unsigned long start = millis();
    while (millis() - start < (unsigned long)notes[i].dur) {
      webServerTick();
      yield();
    }
    ledcWriteTone(BUZZER_PIN, 0);
    delay(20);
  }
  ledcDetach(BUZZER_PIN);
}
