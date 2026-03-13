// ============================================================
//  Animations.ino  —  Chordy Eye Renderer + Buzzer Melodies
//  All drawing is done with Adafruit_SSD1306 + Adafruit_GFX
//  Non-blocking: uses millis() only, never delay()
// ============================================================

#include "Config.h"

// ── Eye layout constants ──────────────────────────────────────
#define EYE_L_CX      36    // Left eye centre X
#define EYE_R_CX      92    // Right eye centre X
#define EYE_CY        30    // Both eyes centre Y
#define EYE_W         26    // Full eye width (ellipse a)
#define EYE_H         22    // Full eye height (ellipse b)
#define PUPIL_R        5    // Pupil radius

// ── Animation state ──────────────────────────────────────────
static AnimID   animCurrent     = ANIM_IDLE;
static int      animFrame       = 0;
static unsigned long animLastMs = 0;
static int      animFrameDelay  = 120;
static bool     animLoop        = true;

// Eye position offsets for look-around
static int eyeOffX = 0;
static int eyeOffY = 0;

// Blink progress  0=open  EYE_H/2=closed
static int blinkH   = EYE_H;   // current drawn height
static bool blinking = false;

// ── Helpers ───────────────────────────────────────────────────

// Draw one rounded-rect "eye" with pupil
void drawEye(int cx, int cy, int w, int h, int pox, int poy, bool filled = false) {
  // Outer white eye
  display.fillRoundRect(cx - w/2, cy - h/2, w, h, h/3, SSD1306_WHITE);
  // Pupil (black)
  int px = cx + pox - PUPIL_R;
  int py = cy + poy - PUPIL_R;
  display.fillCircle(cx + pox, cy + poy, PUPIL_R, SSD1306_BLACK);
  // Shine dot
  display.fillCircle(cx + pox + 2, cy + poy - 2, 1, SSD1306_WHITE);
}

// Eyelid top partial fill (squint / blink)
void drawEyelid(int cx, int cy, int w, int h, int lidH, bool top = true) {
  if (lidH <= 0) return;
  int y = top ? (cy - h/2) : (cy + h/2 - lidH);
  display.fillRect(cx - w/2 - 1, y, w + 2, lidH, SSD1306_BLACK);
}

// Draw both eyes with current offsets
void drawEyes(int pox, int poy, int lidT = 0, int lidB = 0) {
  drawEye(EYE_L_CX, EYE_CY, EYE_W, blinkH, pox, poy);
  drawEye(EYE_R_CX, EYE_CY, EYE_W, blinkH, pox, poy);
  if (lidT > 0) {
    drawEyelid(EYE_L_CX, EYE_CY, EYE_W, blinkH, lidT, true);
    drawEyelid(EYE_R_CX, EYE_CY, EYE_W, blinkH, lidT, true);
  }
  if (lidB > 0) {
    drawEyelid(EYE_L_CX, EYE_CY, EYE_W, blinkH, lidB, false);
    drawEyelid(EYE_R_CX, EYE_CY, EYE_W, blinkH, lidB, false);
  }
}

// ── Heart eye helper ──────────────────────────────────────────
void drawHeart(int cx, int cy, int r) {
  // Simple heart from two circles + triangle
  display.fillCircle(cx - r/2, cy - r/4, r/2, SSD1306_WHITE);
  display.fillCircle(cx + r/2, cy - r/4, r/2, SSD1306_WHITE);
  // Bottom triangle approximation
  for (int i = 0; i <= r; i++) {
    int w2 = r - i;
    display.drawFastHLine(cx - w2, cy - r/4 + i, w2*2, SSD1306_WHITE);
  }
}

// ── Blanket overlay (cold weather) ───────────────────────────
void drawBlanket() {
  // Simple wavy lines at bottom of screen suggesting blanket edge
  for (int x = 0; x < 128; x += 8) {
    display.fillRect(x, 50, 8, 14, (x/8 % 2) ? SSD1306_WHITE : SSD1306_BLACK);
  }
  display.drawRect(0, 50, 128, 14, SSD1306_WHITE);
}

// ── Fan overlay (hot weather) ─────────────────────────────────
void drawFan(int frame) {
  // Spinning fan blades
  int cx = 64, cy = 54, r = 8;
  float angle = frame * 0.3f;
  for (int b = 0; b < 3; b++) {
    float a = angle + b * 2.094f; // 120 deg
    int x1 = cx + (int)(cos(a) * r);
    int y1 = cy + (int)(sin(a) * r);
    int x2 = cx + (int)(cos(a + 1.0f) * r);
    int y2 = cy + (int)(sin(a + 1.0f) * r);
    display.fillTriangle(cx, cy, x1, y1, x2, y2, SSD1306_WHITE);
  }
  // Wind lines
  for (int i = 0; i < 3; i++) {
    int yy = 46 + i * 4;
    display.drawFastHLine(14 + (frame % 4) * 2, yy, 20, SSD1306_WHITE);
    display.drawFastHLine(90 - (frame % 4) * 2, yy, 20, SSD1306_WHITE);
  }
}

// ── Rain overlay ──────────────────────────────────────────────
void drawRain(int frame) {
  for (int i = 0; i < 8; i++) {
    int rx = (i * 17 + frame * 3) % 120 + 4;
    int ry = (frame * 4 + i * 11) % 40 + 2;
    display.drawLine(rx, ry, rx - 2, ry + 5, SSD1306_WHITE);
  }
  // Simple cloud outline at top
  display.drawCircle(32, 5, 6, SSD1306_WHITE);
  display.drawCircle(40, 3, 7, SSD1306_WHITE);
  display.drawCircle(50, 5, 6, SSD1306_WHITE);
  display.drawFastHLine(26, 8, 30, SSD1306_WHITE);
}

// ── Hands / arms overlay ──────────────────────────────────────
void drawHands(int frame) {
  // Wave hands at sides
  int swing = (frame % 10 < 5) ? 3 : -3;
  // Left hand
  display.drawLine(15, 50, 10, 58 + swing, SSD1306_WHITE);
  display.fillCircle(10, 58 + swing, 3, SSD1306_WHITE);
  // Right hand
  display.drawLine(113, 50, 118, 58 + swing, SSD1306_WHITE);
  display.fillCircle(118, 58 + swing, 3, SSD1306_WHITE);
}

// ── Z z z overlay (sleepy) ───────────────────────────────────
void drawZzz(int frame) {
  int offset = (frame * 2) % 20;
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(95, 8 + offset % 10);
  display.print("z");
  display.setCursor(102, 4 + (offset/2) % 8);
  display.print("Z");
  display.setCursor(110, 1 + (offset/3) % 6);
  display.print("Z");
}

// ── Name tag at bottom ────────────────────────────────────────
void drawNameTag() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int len = strlen(config.botName) * 6;
  display.setCursor((OLED_WIDTH - len) / 2, 56);
  display.print(config.botName);
}

// ──────────────────────────────────────────────────────────────
//  animInit — called from setup()
// ──────────────────────────────────────────────────────────────
void animInit() {
  display.clearDisplay();
  display.display();
  blinkH = EYE_H;
  animFrame = 0;
}

// ──────────────────────────────────────────────────────────────
//  animPlay — switch to a new animation
// ──────────────────────────────────────────────────────────────
void animPlay(AnimID id) {
  animCurrent  = id;
  animFrame    = 0;
  animLastMs   = millis();
  blinkH       = EYE_H;
  eyeOffX      = 0;
  eyeOffY      = 0;

  switch (id) {
    case ANIM_BLINK:         animFrameDelay = 40;  animLoop = false; break;
    case ANIM_HAPPY:         animFrameDelay = 80;  animLoop = true;  break;
    case ANIM_VERY_HAPPY:    animFrameDelay = 60;  animLoop = true;  break;
    case ANIM_SCARED:        animFrameDelay = 50;  animLoop = true;  break;
    case ANIM_SLEEPY:        animFrameDelay = 120; animLoop = true;  break;
    case ANIM_SQUINT:        animFrameDelay = 60;  animLoop = false; break;
    case ANIM_HEART_EYES:    animFrameDelay = 80;  animLoop = true;  break;
    case ANIM_LOOK_LEFT:     animFrameDelay = 60;  animLoop = false; break;
    case ANIM_LOOK_RIGHT:    animFrameDelay = 60;  animLoop = false; break;
    case ANIM_WEATHER_COLD:  animFrameDelay = 100; animLoop = true;  break;
    case ANIM_WEATHER_HOT:   animFrameDelay = 80;  animLoop = true;  break;
    case ANIM_WEATHER_RAIN:  animFrameDelay = 80;  animLoop = true;  break;
    case ANIM_STARTUP:       animFrameDelay = 60;  animLoop = false; break;
    default:                 animFrameDelay = 150; animLoop = true;  break;
  }
}

// ──────────────────────────────────────────────────────────────
//  animTick — called every loop(), renders current frame
// ──────────────────────────────────────────────────────────────
void animTick() {
  unsigned long now = millis();
  if (now - animLastMs < (unsigned long)animFrameDelay) return;
  animLastMs = now;

  display.clearDisplay();

  switch (animCurrent) {

    // ── IDLE: gentle breathing blink every ~60 frames ─────────
    case ANIM_IDLE: {
      drawEyes(0, 0);
      drawNameTag();
      // occasional blink
      if (animFrame % 60 == 30) {
        blinkH = max(4, EYE_H - (animFrame % 60 == 30 ? 14 : 0));
      } else {
        blinkH = EYE_H;
      }
      animFrame++;
      break;
    }

    // ── BLINK: quick close and open ───────────────────────────
    case ANIM_BLINK: {
      int seq[] = {EYE_H, 14, 8, 4, 2, 4, 8, 14, EYE_H};
      int n = sizeof(seq)/sizeof(seq[0]);
      if (animFrame < n) {
        blinkH = seq[animFrame];
        drawEyes(0, 0);
        animFrame++;
      } else {
        blinkH = EYE_H;
        drawEyes(0, 0);
        animPlay(ANIM_IDLE);
      }
      break;
    }

    // ── HAPPY: bouncing pupils + smile curve ──────────────────
    case ANIM_HAPPY: {
      int bounce = (animFrame % 10 < 5) ? -2 : 2;
      drawEyes(0, bounce);
      drawHands(animFrame);
      // Smile arc
      display.drawCircle(64, 58, 6, SSD1306_WHITE);
      display.fillRect(58, 52, 12, 8, SSD1306_BLACK);
      drawNameTag();
      animFrame++;
      break;
    }

    // ── VERY HAPPY: heart eyes + bounce ──────────────────────
    case ANIM_VERY_HAPPY: {
      int bounce = (animFrame % 8 < 4) ? -3 : 3;
      // Blank eyes, draw hearts instead
      display.fillRoundRect(EYE_L_CX - EYE_W/2, EYE_CY - EYE_H/2, EYE_W, EYE_H, EYE_H/3, SSD1306_WHITE);
      display.fillRoundRect(EYE_R_CX - EYE_W/2, EYE_CY - EYE_H/2, EYE_W, EYE_H, EYE_H/3, SSD1306_WHITE);
      drawHeart(EYE_L_CX, EYE_CY + bounce, 5);
      drawHeart(EYE_R_CX, EYE_CY + bounce, 5);
      // Invert hearts to black
      display.fillCircle(EYE_L_CX - 3, EYE_CY - 2 + bounce, 3, SSD1306_BLACK);
      display.fillCircle(EYE_L_CX + 3, EYE_CY - 2 + bounce, 3, SSD1306_BLACK);
      display.fillCircle(EYE_R_CX - 3, EYE_CY - 2 + bounce, 3, SSD1306_BLACK);
      display.fillCircle(EYE_R_CX + 3, EYE_CY - 2 + bounce, 3, SSD1306_BLACK);
      drawHands(animFrame);
      drawNameTag();
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_HAPPY);
      break;
    }

    // ── SCARED: wide eyes + shake ─────────────────────────────
    case ANIM_SCARED: {
      int shakeX = (animFrame % 4 < 2) ? -3 : 3;
      blinkH = EYE_H + 4; // wide open
      drawEyes(shakeX, 0);
      // Sweat drop
      display.fillCircle(EYE_R_CX + 16, EYE_CY + 8, 2, SSD1306_WHITE);
      display.drawLine(EYE_R_CX + 16, EYE_CY + 4, EYE_R_CX + 16, EYE_CY + 7, SSD1306_WHITE);
      drawNameTag();
      animFrame++;
      if (animFrame > 60) animPlay(ANIM_IDLE);
      break;
    }

    // ── SLEEPY: half-lid eyes + zzz ───────────────────────────
    case ANIM_SLEEPY: {
      blinkH = EYE_H;
      int lidAmount = EYE_H / 2 + (int)(sin(animFrame * 0.1f) * 2);
      drawEyes(0, 2, lidAmount);
      drawZzz(animFrame);
      drawNameTag();
      animFrame++;
      break;
    }

    // ── SQUINT: bright light ──────────────────────────────────
    case ANIM_SQUINT: {
      int squintSeq[] = {EYE_H, EYE_H/2, EYE_H/3, EYE_H/4, EYE_H/3, EYE_H/2, EYE_H};
      int n = sizeof(squintSeq)/sizeof(squintSeq[0]);
      if (animFrame < n) {
        blinkH = squintSeq[animFrame];
        drawEyes(0, 0, blinkH > 4 ? (EYE_H - blinkH)/2 : 0);
        animFrame++;
      } else {
        blinkH = EYE_H;
        drawEyes(0, 0);
        animPlay(ANIM_IDLE);
      }
      break;
    }

    // ── HEART EYES (pet) ──────────────────────────────────────
    case ANIM_HEART_EYES: {
      animPlay(ANIM_VERY_HAPPY); // reuse very happy
      break;
    }

    // ── LOOK LEFT ─────────────────────────────────────────────
    case ANIM_LOOK_LEFT: {
      int pSeq[] = {0, -3, -5, -5, -5, -3, 0};
      int n = sizeof(pSeq)/sizeof(pSeq[0]);
      if (animFrame < n) {
        drawEyes(pSeq[animFrame], 0);
        animFrame++;
      } else {
        animPlay(ANIM_IDLE);
      }
      break;
    }

    // ── LOOK RIGHT ────────────────────────────────────────────
    case ANIM_LOOK_RIGHT: {
      int pSeq[] = {0, 3, 5, 5, 5, 3, 0};
      int n = sizeof(pSeq)/sizeof(pSeq[0]);
      if (animFrame < n) {
        drawEyes(pSeq[animFrame], 0);
        animFrame++;
      } else {
        animPlay(ANIM_IDLE);
      }
      break;
    }

    // ── WEATHER COLD: blanket + shiver ────────────────────────
    case ANIM_WEATHER_COLD: {
      int shiverX = (animFrame % 4 < 2) ? -1 : 1;
      blinkH = EYE_H - 2;
      drawEyes(shiverX, 0);
      drawBlanket();
      // Status text
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(2, 2);
      display.printf("%.0fC Brr..", weatherData.tempC);
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    // ── WEATHER HOT: fan + sweaty ─────────────────────────────
    case ANIM_WEATHER_HOT: {
      drawEyes(0, 1);
      drawFan(animFrame);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(2, 2);
      display.printf("%.0fC Hot!", weatherData.tempC);
      // Sweat drops
      display.fillCircle(EYE_L_CX - 12, EYE_CY + 10 + (animFrame % 6), 2, SSD1306_WHITE);
      display.fillCircle(EYE_R_CX + 12, EYE_CY + 10 + (animFrame % 5), 2, SSD1306_WHITE);
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    // ── WEATHER RAIN ──────────────────────────────────────────
    case ANIM_WEATHER_RAIN: {
      drawEyes(0, 0);
      drawRain(animFrame);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(2, 52);
      display.print("Rainy :(");
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    // ── STARTUP: eyes appear from centre ─────────────────────
    case ANIM_STARTUP: {
      int growSeq[] = {2, 6, 10, 14, 18, EYE_H, EYE_H};
      int n = sizeof(growSeq)/sizeof(growSeq[0]);
      if (animFrame < n) {
        blinkH = growSeq[animFrame];
        drawEyes(0, 0);
        animFrame++;
      } else {
        blinkH = EYE_H;
        drawEyes(0, 0);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(32, 54);
        display.print("Hi! I'm Chordy");
        animPlay(ANIM_IDLE);
      }
      break;
    }

    default:
      drawEyes(0, 0);
      drawNameTag();
      break;
  }

  display.display();
}

// ──────────────────────────────────────────────────────────────
//  Buzzer helpers
// ──────────────────────────────────────────────────────────────
void buzzerTone(int freq, int durationMs) {
  ledcSetup(0, freq, 8);
  ledcAttachPin(BUZZER_PIN, 0);
  ledcWrite(0, 128);
  unsigned long start = millis();
  while (millis() - start < (unsigned long)durationMs) {
    webServerTick(); // keep network alive during tone
  }
  ledcWrite(0, 0);
}

// Melody IDs:
// 0 = startup jingle   1 = happy           2 = timer set
// 3 = sleep            4 = timer alarm      5 = scared
void buzzerMelody(int id) {
  struct Note { int freq; int dur; };

  const Note startup[]  = {{523,80},{659,80},{784,120},{1047,200},{0,50},{1047,80}};
  const Note happy[]    = {{784,80},{880,80},{988,160}};
  const Note timerSet[] = {{880,60},{1047,100}};
  const Note sleep[]    = {{523,200},{440,200},{349,300}};
  const Note alarm[]    = {{1047,100},{0,50},{1047,100},{0,50},{1047,300}};
  const Note scared[]   = {{200,60},{180,60},{160,100},{0,30},{160,80}};

  const Note* notes = nullptr;
  int len = 0;
  switch(id) {
    case 0: notes = startup;  len = 6; break;
    case 1: notes = happy;    len = 3; break;
    case 2: notes = timerSet; len = 2; break;
    case 3: notes = sleep;    len = 3; break;
    case 4: notes = alarm;    len = 5; break;
    case 5: notes = scared;   len = 5; break;
  }
  if (!notes) return;

  ledcSetup(0, 1000, 8);
  ledcAttachPin(BUZZER_PIN, 0);
  for (int i = 0; i < len; i++) {
    if (notes[i].freq == 0) {
      ledcWrite(0, 0);
    } else {
      ledcWriteTone(0, notes[i].freq);
      ledcWrite(0, 128);
    }
    unsigned long s = millis();
    while (millis() - s < (unsigned long)notes[i].dur) webServerTick();
  }
  ledcWrite(0, 0);
}
