// ============================================================
//  Animations.ino  —  Chordy v2.0
//  Fixed hands/arms, new emotions, eye styles, speed control,
//  timer border, better idle motions, buzzer for all emotions
// ============================================================
#include "Config.h"
#include <math.h>

// ── Eye layout (base values — scaled by eye style) ───────────
#define EYE_L_CX   36
#define EYE_R_CX   92
#define EYE_CY     28
#define BASE_EYE_W 26
#define BASE_EYE_H 20
#define PUPIL_R     5

// ── Anim state ────────────────────────────────────────────────
static AnimID        animCurrent    = ANIM_IDLE;
static int           animFrame      = 0;
static unsigned long animLastMs     = 0;
static int           animFrameDelay = 120;
static bool          animLoop       = true;
static int           blinkH         = BASE_EYE_H;

// ── Runtime eye size (set by eye style) ──────────────────────
static int EYE_W = BASE_EYE_W;
static int EYE_H = BASE_EYE_H;

extern CConfig    config;
extern bool       timerActive;
extern unsigned long timerEndMs;
extern unsigned long timerTotalMs;

// ──────────────────────────────────────────────────────────────
//  Eye style selector
// ──────────────────────────────────────────────────────────────
void applyEyeStyle() {
  switch (config.eyeStyle) {
    case EYE_OVAL:  EYE_W = 22; EYE_H = 26; break;  // tall oval
    case EYE_CUTE:  EYE_W = 20; EYE_H = 20; break;  // round cute
    case EYE_WIDE:  EYE_W = 30; EYE_H = 18; break;  // wide
    default:        EYE_W = BASE_EYE_W; EYE_H = BASE_EYE_H; break;
  }
  blinkH = EYE_H;
}

// ── Speed helper — maps config.animSpeed (1-10) to frame delay
int speedToDelay(int baseMs) {
  float s = constrain(config.animSpeed, 1, 10);
  float factor = 2.0f - (s / 10.0f) * 1.6f; // 1.84 (slow) .. 0.24 (fast)
  return max(20, (int)(baseMs * factor));
}

// ──────────────────────────────────────────────────────────────
//  Core drawing helpers
// ──────────────────────────────────────────────────────────────
void drawSingleEye(int cx, int cy, int w, int h, int pox, int poy) {
  if (h < 2) return;
  display.fillRoundRect(cx - w/2, cy - h/2, w, h, h/3 + 1, SSD1306_WHITE);
  // Pupil
  int px = constrain(cx + pox, cx - w/2 + PUPIL_R + 1, cx + w/2 - PUPIL_R - 1);
  int py = constrain(cy + poy, cy - h/2 + PUPIL_R + 1, cy + h/2 - PUPIL_R - 1);
  display.fillCircle(px, py, PUPIL_R, SSD1306_BLACK);
  // Shine
  display.fillCircle(px + 2, py - 2, 1, SSD1306_WHITE);
}

void drawEyes(int pox, int poy, int topLid = 0) {
  drawSingleEye(EYE_L_CX, EYE_CY, EYE_W, blinkH, pox, poy);
  drawSingleEye(EYE_R_CX, EYE_CY, EYE_W, blinkH, pox, poy);
  if (topLid > 0) {
    display.fillRect(EYE_L_CX - EYE_W/2 - 1, EYE_CY - EYE_H/2, EYE_W + 2, topLid, SSD1306_BLACK);
    display.fillRect(EYE_R_CX - EYE_W/2 - 1, EYE_CY - EYE_H/2, EYE_W + 2, topLid, SSD1306_BLACK);
  }
}

// Draw asymmetric eye for wink
void drawEyeOne(int side, int pox, int poy, bool closed) {
  int cx = (side == 0) ? EYE_L_CX : EYE_R_CX;
  if (closed) {
    // Just a horizontal line
    display.drawFastHLine(cx - EYE_W/2, EYE_CY, EYE_W, SSD1306_WHITE);
    display.drawFastHLine(cx - EYE_W/2, EYE_CY + 1, EYE_W, SSD1306_WHITE);
  } else {
    drawSingleEye(cx, EYE_CY, EYE_W, blinkH, pox, poy);
  }
}

void drawNameTag() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int len = strlen(config.botName) * 6;
  display.setCursor((128 - len) / 2, 56);
  display.print(config.botName);
}

// ── Heart helper ──────────────────────────────────────────────
void drawHeart(int cx, int cy, int r) {
  display.fillCircle(cx - r/2, cy - r/3, r/2, SSD1306_WHITE);
  display.fillCircle(cx + r/2, cy - r/3, r/2, SSD1306_WHITE);
  for (int i = 0; i <= r; i++) {
    int hw = r - i + 1;
    display.drawFastHLine(cx - hw, cy - r/3 + i, hw * 2, SSD1306_WHITE);
  }
}

// ── Properly drawn arms/hands ──────────────────────────────────
// side: 0=left arm, 1=right arm; raise: 0=down, 10=up
void drawArm(int side, int raise, int wave) {
  int shoulderX = (side == 0) ? EYE_L_CX - EYE_W/2 - 2 : EYE_R_CX + EYE_W/2 + 2;
  int shoulderY = EYE_CY + 8;
  int dir = (side == 0) ? -1 : 1;

  // Upper arm
  int elbowX = shoulderX + dir * (8 + wave/2);
  int elbowY = shoulderY + (12 - raise/2);
  display.drawLine(shoulderX, shoulderY, elbowX, elbowY, SSD1306_WHITE);

  // Forearm  
  int handX = elbowX + dir * 6;
  int handY = elbowY - (2 + wave);
  display.drawLine(elbowX, elbowY, handX, handY, SSD1306_WHITE);

  // Hand — small circle
  display.fillCircle(handX, handY, 3, SSD1306_WHITE);
}

// ── Mouth expressions ─────────────────────────────────────────
void drawSmile(int cx, int y, int w, int h) {
  // Draw a smile arc using points
  for (int i = -w/2; i <= w/2; i++) {
    int dy = (int)(h * sin((float)(i + w/2) / w * 3.14159f));
    display.drawPixel(cx + i, y + dy, SSD1306_WHITE);
    display.drawPixel(cx + i, y + dy + 1, SSD1306_WHITE);
  }
}

void drawFrown(int cx, int y, int w, int h) {
  for (int i = -w/2; i <= w/2; i++) {
    int dy = (int)(h * sin((float)(i + w/2) / w * 3.14159f));
    display.drawPixel(cx + i, y + dy, SSD1306_WHITE);
  }
}

// ── Weather overlays ──────────────────────────────────────────
void drawBlanket() {
  for (int x = 0; x < 128; x += 10) {
    display.fillRect(x, 50, 10, 14, (x/10 % 2) ? SSD1306_WHITE : SSD1306_BLACK);
  }
  display.drawRect(0, 50, 128, 14, SSD1306_WHITE);
  // Zigzag top edge
  for (int x = 0; x < 128; x += 4) {
    display.drawPixel(x,   50, SSD1306_WHITE);
    display.drawPixel(x+1, 49, SSD1306_WHITE);
    display.drawPixel(x+2, 50, SSD1306_WHITE);
    display.drawPixel(x+3, 51, SSD1306_WHITE);
  }
}

void drawFan(int frame) {
  int cx = 64, cy = 55, r = 7;
  float angle = frame * 0.4f;
  for (int b = 0; b < 4; b++) {
    float a = angle + b * 1.5708f;
    int x1 = cx + (int)(cos(a) * r);
    int y1 = cy + (int)(sin(a) * r);
    int x2 = cx + (int)(cos(a + 0.7f) * r);
    int y2 = cy + (int)(sin(a + 0.7f) * r);
    display.fillTriangle(cx, cy, x1, y1, x2, y2, SSD1306_WHITE);
  }
  display.fillCircle(cx, cy, 2, SSD1306_WHITE);
  // Wind lines
  for (int i = 0; i < 3; i++) {
    int yy = 46 + i * 4;
    int off = (frame * 2 + i * 5) % 20;
    display.drawFastHLine(6 + off, yy, 14, SSD1306_WHITE);
    display.drawFastHLine(90 - off, yy, 14, SSD1306_WHITE);
  }
}

void drawRain(int frame) {
  for (int i = 0; i < 8; i++) {
    int rx = (i * 19 + frame * 4) % 116 + 6;
    int ry = (frame * 5 + i * 13) % 38 + 2;
    display.drawLine(rx, ry, rx - 2, ry + 5, SSD1306_WHITE);
  }
  // Cloud
  display.fillCircle(30,  6, 7, SSD1306_WHITE);
  display.fillCircle(40,  4, 8, SSD1306_WHITE);
  display.fillCircle(52,  6, 7, SSD1306_WHITE);
  display.fillRect(24, 6, 35, 8, SSD1306_WHITE);
}

// ── Zzz overlay ───────────────────────────────────────────────
void drawZzz(int frame) {
  int off = (frame / 4) % 12;
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(96, 8 + off % 8);   display.print("z");
  display.setCursor(104, 4 + off % 6);  display.print("Z");
  display.setCursor(112, 1 + off % 4);  display.print("Z");
}

// ── Timer border ─────────────────────────────────────────────
void drawTimerBorder() {
  if (!timerActive || timerTotalMs == 0) return;
  unsigned long elapsed = millis() - (timerEndMs - timerTotalMs);
  float ratio = 1.0f - constrain((float)elapsed / timerTotalMs, 0.0f, 1.0f);
  // Total perimeter = 2*(128+64) = 384 pixels
  int total = 384;
  int active = (int)(ratio * total);
  int x = 0, y = 0;
  // Draw border segments clockwise: top, right, bottom, left
  // Top (0..127)
  int seg = min(active, 128);
  if (seg > 0) display.drawFastHLine(0, 0, seg, SSD1306_WHITE);
  active -= seg;
  if (active <= 0) return;
  // Right (0..63)
  seg = min(active, 64);
  if (seg > 0) display.drawFastVLine(127, 0, seg, SSD1306_WHITE);
  active -= seg;
  if (active <= 0) return;
  // Bottom right to left (0..127)
  seg = min(active, 128);
  if (seg > 0) display.drawFastHLine(127 - seg, 63, seg, SSD1306_WHITE);
  active -= seg;
  if (active <= 0) return;
  // Left (bottom to top)
  seg = min(active, 64);
  if (seg > 0) display.drawFastVLine(0, 63 - seg, seg, SSD1306_WHITE);
}

// ──────────────────────────────────────────────────────────────
//  NEW EMOTION HELPERS
// ──────────────────────────────────────────────────────────────

// Guitar: simple guitar outline at bottom + strum hand
void drawGuitar(int frame) {
  int gy = 48;
  // Body
  display.drawRoundRect(50, gy, 28, 14, 6, SSD1306_WHITE);
  // Neck
  display.drawLine(50, gy + 4, 28, gy - 4, SSD1306_WHITE);
  display.drawLine(50, gy + 8, 28, gy,     SSD1306_WHITE);
  // Strings (3)
  for (int s = 0; s < 3; s++) {
    display.drawLine(32, gy - 4 + s*2, 72, gy + 2 + s*2, SSD1306_WHITE);
  }
  // Strum hand  
  int strumX = 65 + (frame % 6 < 3 ? -2 : 2);
  display.fillCircle(strumX, gy + 5, 3, SSD1306_WHITE);
  // Musical notes
  int noteOff = (frame * 3) % 30;
  display.setCursor(85, 48 - noteOff % 15);
  display.setTextSize(1);
  display.print("*");
  display.setCursor(90, 44 - noteOff % 10);
  display.print("~");
}

// Whistle: puffy cheeks + "O" lips + music notes
void drawWhistle(int frame) {
  // Cheeks
  int puff = (frame % 8 < 4) ? 1 : 0;
  display.fillCircle(EYE_L_CX, EYE_CY + 20 + puff, 6, SSD1306_WHITE);
  display.fillCircle(EYE_R_CX, EYE_CY + 20 + puff, 6, SSD1306_WHITE);
  // Mouth "O"
  display.drawCircle(64, EYE_CY + 22, 4, SSD1306_WHITE);
  // Notes flying out
  int noteX = 75 + (frame * 4) % 40;
  display.setTextSize(1);
  display.setCursor(noteX, 38);
  display.print("♪");
  if (noteX + 8 < 128) { display.setCursor(noteX + 8, 34); display.print("♩"); }
}

// Fly tracking: one eye looks at a small fly dot that moves
void drawFlyTracking(int frame) {
  float t = frame * 0.15f;
  int flyX = 64 + (int)(50 * cos(t * 1.3f));
  int flyY = 20 + (int)(15 * sin(t * 0.8f));
  flyX = constrain(flyX, 4, 124);
  flyY = constrain(flyY, 4, 50);

  // Fly dot
  display.fillCircle(flyX, flyY, 2, SSD1306_WHITE);
  // Tiny wings
  display.drawLine(flyX - 4, flyY - 1, flyX - 1, flyY, SSD1306_WHITE);
  display.drawLine(flyX + 1, flyY,     flyX + 4, flyY - 1, SSD1306_WHITE);

  // Eyes both tracking fly
  int dx = flyX - 64;
  int dy = flyY - EYE_CY;
  int pox = constrain(dx / 6, -5, 5);
  int poy = constrain(dy / 5, -4, 4);
  drawEyes(pox, poy);

  // "?" above head
  if (frame % 20 < 10) {
    display.setTextSize(1);
    display.setCursor(56, 50);
    display.print("?");
  }
}

// Typing: rapid pupil flicker left-right, keyboard drawn below
void drawTyping(int frame) {
  int pox = (frame % 4 < 2) ? -3 : 3;
  drawEyes(pox, 2);
  // Keyboard rows
  int ky = 50;
  for (int r = 0; r < 2; r++) {
    for (int c = 0; c < 8; c++) {
      display.drawRect(c * 15 + 4, ky + r * 7, 12, 5, SSD1306_WHITE);
    }
  }
  // Animated "key press"
  int pressedKey = (frame / 3) % 8;
  display.fillRect(pressedKey * 15 + 4, ky, 12, 5, SSD1306_WHITE);
}

// Coffee: cup with steam + eyes half-lidded
void drawCoffee(int frame) {
  // Cup
  display.drawRoundRect(52, 48, 24, 14, 3, SSD1306_WHITE);
  display.drawFastHLine(48, 50, 4, SSD1306_WHITE); // handle top
  display.drawFastHLine(48, 56, 4, SSD1306_WHITE); // handle bot
  display.drawFastVLine(48, 50, 7, SSD1306_WHITE); // handle side
  display.fillRect(52, 57, 24, 5, SSD1306_WHITE);  // filled bottom
  // Steam wisps
  int s = (frame * 2) % 16;
  display.drawLine(60, 48, 58, 40 - s, SSD1306_WHITE);
  display.drawLine(64, 48, 64, 40 - s, SSD1306_WHITE);
  display.drawLine(68, 48, 70, 40 - s, SSD1306_WHITE);
  // Contented half-lid eyes
  int lid = EYE_H / 3;
  drawEyes(0, 0, lid);
}

// Magnifying glass: one eye enlarged, glass shape overlaid
void drawMagnify(int frame) {
  // Larger left eye
  display.fillRoundRect(EYE_L_CX - EYE_W/2 - 4, EYE_CY - EYE_H/2 - 4,
                        EYE_W + 8, EYE_H + 8, EYE_H/2, SSD1306_WHITE);
  display.fillCircle(EYE_L_CX, EYE_CY, PUPIL_R + 2, SSD1306_BLACK);
  display.fillCircle(EYE_L_CX + 3, EYE_CY - 3, 2, SSD1306_WHITE);
  // Normal right eye
  drawSingleEye(EYE_R_CX, EYE_CY, EYE_W, blinkH, 0, 0);
  // Glass handle
  display.drawLine(EYE_L_CX - EYE_W/2 - 4 + 2, EYE_CY + EYE_H/2 + 4,
                   EYE_L_CX - EYE_W/2 - 10, EYE_CY + EYE_H/2 + 12, SSD1306_WHITE);
  display.drawLine(EYE_L_CX - EYE_W/2 - 3 + 2, EYE_CY + EYE_H/2 + 4,
                   EYE_L_CX - EYE_W/2 - 9, EYE_CY + EYE_H/2 + 12, SSD1306_WHITE);
  // Magnify circle
  display.drawCircle(EYE_L_CX, EYE_CY, EYE_W/2 + 5, SSD1306_WHITE);
}

// Thinking: look up-right, "..." bubble
void drawThinking(int frame) {
  int pox = 3;
  int poy = -3;
  // Slow drift
  pox += (int)(sin(frame * 0.05f) * 2);
  drawEyes(pox, poy);
  // Thought bubble dots
  int dx = EYE_R_CX + EYE_W/2 + 4;
  int dy = EYE_CY - EYE_H/2 - 2;
  display.fillCircle(dx,     dy,     2, SSD1306_WHITE);
  display.fillCircle(dx + 5, dy - 3, 3, SSD1306_WHITE);
  display.fillCircle(dx + 12,dy - 7, 4, SSD1306_WHITE);
  // "..." or question mark inside bubble
  display.setTextSize(1);
  display.setCursor(dx + 8, dy - 12);
  if (frame % 40 < 20) display.print("?");
  else                  display.print("!");
}

// ──────────────────────────────────────────────────────────────
//  animInit / animPlay / animTick
// ──────────────────────────────────────────────────────────────
void animInit() {
  applyEyeStyle();
  blinkH = EYE_H;
  display.clearDisplay();
  display.display();
}

void animPlay(AnimID id) {
  applyEyeStyle();
  animCurrent  = id;
  animFrame    = 0;
  animLastMs   = millis();
  blinkH       = EYE_H;

  switch (id) {
    case ANIM_BLINK:         animFrameDelay = speedToDelay(35);  animLoop = false; break;
    case ANIM_HAPPY:         animFrameDelay = speedToDelay(80);  animLoop = true;  break;
    case ANIM_VERY_HAPPY:    animFrameDelay = speedToDelay(65);  animLoop = true;  break;
    case ANIM_SCARED:        animFrameDelay = speedToDelay(45);  animLoop = true;  break;
    case ANIM_SLEEPY:        animFrameDelay = speedToDelay(130); animLoop = true;  break;
    case ANIM_SQUINT:        animFrameDelay = speedToDelay(55);  animLoop = false; break;
    case ANIM_HEART_EYES:    animFrameDelay = speedToDelay(75);  animLoop = true;  break;
    case ANIM_LOOK_LEFT:
    case ANIM_LOOK_RIGHT:
    case ANIM_LOOK_UP:
    case ANIM_LOOK_DOWN:     animFrameDelay = speedToDelay(55);  animLoop = false; break;
    case ANIM_WINK:          animFrameDelay = speedToDelay(50);  animLoop = false; break;
    case ANIM_THINKING:      animFrameDelay = speedToDelay(90);  animLoop = true;  break;
    case ANIM_WEATHER_COLD:  animFrameDelay = speedToDelay(95);  animLoop = true;  break;
    case ANIM_WEATHER_HOT:   animFrameDelay = speedToDelay(75);  animLoop = true;  break;
    case ANIM_WEATHER_RAIN:  animFrameDelay = speedToDelay(75);  animLoop = true;  break;
    case ANIM_PLAYING_GUITAR:animFrameDelay = speedToDelay(90);  animLoop = true;  break;
    case ANIM_WHISTLING:     animFrameDelay = speedToDelay(85);  animLoop = true;  break;
    case ANIM_FLY:           animFrameDelay = speedToDelay(70);  animLoop = true;  break;
    case ANIM_TYPING:        animFrameDelay = speedToDelay(60);  animLoop = true;  break;
    case ANIM_COFFEE:        animFrameDelay = speedToDelay(100); animLoop = true;  break;
    case ANIM_MAGNIFY:       animFrameDelay = speedToDelay(90);  animLoop = true;  break;
    case ANIM_STARTUP:       animFrameDelay = speedToDelay(55);  animLoop = false; break;
    default:                 animFrameDelay = speedToDelay(150); animLoop = true;  break;
  }
}

void animTick() {
  unsigned long now = millis();
  if (now - animLastMs < (unsigned long)animFrameDelay) return;
  animLastMs = now;

  display.clearDisplay();

  switch (animCurrent) {

    // ── IDLE: gentle drift + varied blink ─────────────────────
    case ANIM_IDLE: {
      // Gentle eye drift
      float t = animFrame * 0.03f;
      int pox = (int)(sin(t) * 3);
      int poy = (int)(cos(t * 0.7f) * 2);
      // Eye size micro variation (one eye slightly different)
      int leftH  = blinkH + (int)(sin(t * 1.2f) * 1);
      int rightH = blinkH + (int)(cos(t * 0.9f) * 1);
      leftH  = max(8, leftH);
      rightH = max(8, rightH);
      // Draw each eye individually for asymmetry
      blinkH = leftH;
      drawSingleEye(EYE_L_CX, EYE_CY, EYE_W, leftH,  pox, poy);
      blinkH = rightH;
      drawSingleEye(EYE_R_CX, EYE_CY, EYE_W, rightH, pox, poy);
      blinkH = EYE_H;
      drawNameTag();
      drawTimerBorder();
      animFrame++;
      break;
    }

    // ── BLINK ─────────────────────────────────────────────────
    case ANIM_BLINK: {
      int seq[] = {EYE_H, EYE_H*3/4, EYE_H/2, EYE_H/4, 2, 2, EYE_H/4, EYE_H/2, EYE_H};
      int n = sizeof(seq)/sizeof(seq[0]);
      if (animFrame < n) {
        blinkH = seq[animFrame];
        drawEyes(0, 0);
        animFrame++;
      } else {
        blinkH = EYE_H;
        drawEyes(0, 0);
        drawNameTag();
        animPlay(ANIM_IDLE);
        return;
      }
      break;
    }

    // ── HAPPY: arms wave ──────────────────────────────────────
    case ANIM_HAPPY: {
      int bounce = (animFrame % 10 < 5) ? -2 : 2;
      drawEyes(0, bounce);
      drawSmile(64, EYE_CY + EYE_H/2 + 12, 16, 4);
      int wave = (animFrame % 12 < 6) ? 4 : -2;
      drawArm(0, 5, wave);
      drawArm(1, 5, wave);
      drawNameTag();
      drawTimerBorder();
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    // ── VERY HAPPY: heart eyes + arms up ─────────────────────
    case ANIM_VERY_HAPPY: {
      int bounce = (animFrame % 8 < 4) ? -3 : 3;
      // White eye bg
      display.fillRoundRect(EYE_L_CX - EYE_W/2, EYE_CY - EYE_H/2, EYE_W, EYE_H, EYE_H/3, SSD1306_WHITE);
      display.fillRoundRect(EYE_R_CX - EYE_W/2, EYE_CY - EYE_H/2, EYE_W, EYE_H, EYE_H/3, SSD1306_WHITE);
      // Hearts inside eyes (black on white — draw then invert region)
      drawHeart(EYE_L_CX, EYE_CY + bounce/2, 5);
      drawHeart(EYE_R_CX, EYE_CY + bounce/2, 5);
      // Mask to invert hearts to black
      display.fillCircle(EYE_L_CX - 3, EYE_CY - 2 + bounce/2, 2, SSD1306_BLACK);
      display.fillCircle(EYE_L_CX + 3, EYE_CY - 2 + bounce/2, 2, SSD1306_BLACK);
      display.fillCircle(EYE_R_CX - 3, EYE_CY - 2 + bounce/2, 2, SSD1306_BLACK);
      display.fillCircle(EYE_R_CX + 3, EYE_CY - 2 + bounce/2, 2, SSD1306_BLACK);
      // Arms up waving
      int wave = (animFrame % 10 < 5) ? 6 : 2;
      drawArm(0, 8, wave);
      drawArm(1, 8, wave);
      drawSmile(64, EYE_CY + EYE_H/2 + 10, 14, 3);
      drawTimerBorder();
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_HAPPY);
      break;
    }

    // ── HEART EYES (pet) ──────────────────────────────────────
    case ANIM_HEART_EYES: {
      // Reuse very happy
      animPlay(ANIM_VERY_HAPPY);
      return;
    }

    // ── SCARED ────────────────────────────────────────────────
    case ANIM_SCARED: {
      int shakeX = (animFrame % 4 < 2) ? -3 : 3;
      blinkH = min(EYE_H + 5, 28); // wide eyes
      drawEyes(shakeX, 0);
      // Eyebrows arch upward
      display.drawLine(EYE_L_CX - EYE_W/2, EYE_CY - EYE_H/2 - 4,
                       EYE_L_CX + EYE_W/2, EYE_CY - EYE_H/2 - 4, SSD1306_WHITE);
      display.drawLine(EYE_R_CX - EYE_W/2, EYE_CY - EYE_H/2 - 4,
                       EYE_R_CX + EYE_W/2, EYE_CY - EYE_H/2 - 4, SSD1306_WHITE);
      // Sweat drop
      display.drawLine(EYE_R_CX + 16, EYE_CY - 2, EYE_R_CX + 16, EYE_CY + 5, SSD1306_WHITE);
      display.fillCircle(EYE_R_CX + 16, EYE_CY + 7, 2, SSD1306_WHITE);
      drawFrown(64, EYE_CY + EYE_H/2 + 12, 12, 3);
      blinkH = EYE_H;
      animFrame++;
      break; // LDR reaction timeout handles the return
    }

    // ── SLEEPY ────────────────────────────────────────────────
    case ANIM_SLEEPY: {
      int lid = EYE_H / 2 + (int)(sin(animFrame * 0.08f) * 2);
      blinkH = EYE_H;
      drawEyes(0, 3, lid);
      drawZzz(animFrame);
      drawNameTag();
      animFrame++;
      break;
    }

    // ── SQUINT ────────────────────────────────────────────────
    case ANIM_SQUINT: {
      int squintSeq[] = {EYE_H, EYE_H*2/3, EYE_H/3, EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6,EYE_H/6, EYE_H/3, EYE_H*2/3, EYE_H};
      int n = sizeof(squintSeq)/sizeof(squintSeq[0]);
      if (animFrame < n) {
        blinkH = max(2, squintSeq[animFrame]);
        int lid = (EYE_H - blinkH) / 2;
        drawEyes(0, 0, lid);
        animFrame++;
      } else {
        blinkH = EYE_H;
        drawEyes(0, 0);
        animPlay(ANIM_IDLE);
        return;
      }
      break;
    }

    // ── LOOK LEFT ─────────────────────────────────────────────
    case ANIM_LOOK_LEFT: {
      int seq[] = {0, -2, -4, -5, -5, -5, -5, -3, 0};
      int n = sizeof(seq)/sizeof(seq[0]);
      if (animFrame < n) { drawEyes(seq[animFrame], 0); animFrame++; }
      else { drawEyes(0, 0); animPlay(ANIM_IDLE); return; }
      break;
    }

    // ── LOOK RIGHT ────────────────────────────────────────────
    case ANIM_LOOK_RIGHT: {
      int seq[] = {0, 2, 4, 5, 5, 5, 5, 3, 0};
      int n = sizeof(seq)/sizeof(seq[0]);
      if (animFrame < n) { drawEyes(seq[animFrame], 0); animFrame++; }
      else { drawEyes(0, 0); animPlay(ANIM_IDLE); return; }
      break;
    }

    // ── LOOK UP ───────────────────────────────────────────────
    case ANIM_LOOK_UP: {
      int seq[] = {0, -2, -4, -4, -4, -2, 0};
      int n = sizeof(seq)/sizeof(seq[0]);
      if (animFrame < n) { drawEyes(0, seq[animFrame]); animFrame++; }
      else { drawEyes(0, 0); animPlay(ANIM_IDLE); return; }
      break;
    }

    // ── LOOK DOWN ─────────────────────────────────────────────
    case ANIM_LOOK_DOWN: {
      int seq[] = {0, 2, 4, 4, 4, 2, 0};
      int n = sizeof(seq)/sizeof(seq[0]);
      if (animFrame < n) { drawEyes(0, seq[animFrame]); animFrame++; }
      else { drawEyes(0, 0); animPlay(ANIM_IDLE); return; }
      break;
    }

    // ── WINK ──────────────────────────────────────────────────
    case ANIM_WINK: {
      int seq[] = {0, 1, 2, 3, 3, 3, 2, 1, 0};  // wink phase
      int n = sizeof(seq)/sizeof(seq[0]);
      if (animFrame < n) {
        bool winked = seq[animFrame] >= 2;
        drawEyeOne(0, 0, 0, winked);
        drawSingleEye(EYE_R_CX, EYE_CY, EYE_W, blinkH, 0, 0);
        animFrame++;
      } else {
        drawEyes(0, 0);
        animPlay(ANIM_IDLE);
        return;
      }
      break;
    }

    // ── THINKING ──────────────────────────────────────────────
    case ANIM_THINKING: {
      drawThinking(animFrame);
      drawTimerBorder();
      animFrame++;
      if (animFrame > 60) animPlay(ANIM_IDLE);
      break;
    }

    // ── WEATHER COLD ──────────────────────────────────────────
    case ANIM_WEATHER_COLD: {
      int shiver = (animFrame % 4 < 2) ? -1 : 1;
      blinkH = EYE_H - 2;
      drawEyes(shiver, 0);
      drawBlanket();
      display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
      display.setCursor(2, 2);
      display.printf("%.0fC  Brr..", weatherData.tempC);
      blinkH = EYE_H;
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    // ── WEATHER HOT ───────────────────────────────────────────
    case ANIM_WEATHER_HOT: {
      drawEyes(0, 1);
      drawFan(animFrame);
      display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
      display.setCursor(2, 2);
      display.printf("%.0fC  Hot!", weatherData.tempC);
      display.fillCircle(EYE_L_CX - 14, EYE_CY + 10 + animFrame % 6, 2, SSD1306_WHITE);
      display.fillCircle(EYE_R_CX + 14, EYE_CY + 10 + animFrame % 5, 2, SSD1306_WHITE);
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    // ── WEATHER RAIN ──────────────────────────────────────────
    case ANIM_WEATHER_RAIN: {
      drawEyes(0, 0);
      drawRain(animFrame);
      display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
      display.setCursor(2, 54);
      display.print("Rainy day :(");
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    // ── PLAYING GUITAR ────────────────────────────────────────
    case ANIM_PLAYING_GUITAR: {
      // Focused happy eyes
      int pox = (animFrame % 6 < 3) ? 1 : -1;
      drawEyes(pox, 0);
      drawSmile(64, EYE_CY + EYE_H/2 + 12, 12, 3);
      drawGuitar(animFrame);
      animFrame++;
      if (animFrame > 90) animPlay(ANIM_IDLE);
      break;
    }

    // ── WHISTLING ─────────────────────────────────────────────
    case ANIM_WHISTLING: {
      drawEyes(0, -1);
      drawWhistle(animFrame);
      animFrame++;
      if (animFrame > 70) animPlay(ANIM_IDLE);
      break;
    }

    // ── FLY TRACKING ──────────────────────────────────────────
    case ANIM_FLY: {
      drawFlyTracking(animFrame);
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    // ── TYPING ────────────────────────────────────────────────
    case ANIM_TYPING: {
      drawTyping(animFrame);
      animFrame++;
      if (animFrame > 70) animPlay(ANIM_IDLE);
      break;
    }

    // ── COFFEE ────────────────────────────────────────────────
    case ANIM_COFFEE: {
      drawCoffee(animFrame);
      drawTimerBorder();
      animFrame++;
      if (animFrame > 80) animPlay(ANIM_IDLE);
      break;
    }

    // ── MAGNIFY ───────────────────────────────────────────────
    case ANIM_MAGNIFY: {
      drawMagnify(animFrame);
      animFrame++;
      if (animFrame > 70) animPlay(ANIM_IDLE);
      break;
    }

    // ── STARTUP ───────────────────────────────────────────────
    case ANIM_STARTUP: {
      int seq[] = {2, 5, 8, 12, 16, EYE_H, EYE_H};
      int n = sizeof(seq)/sizeof(seq[0]);
      if (animFrame < n) {
        blinkH = seq[animFrame];
        drawSingleEye(EYE_L_CX, EYE_CY, EYE_W, blinkH, 0, 0);
        drawSingleEye(EYE_R_CX, EYE_CY, EYE_W, blinkH, 0, 0);
        animFrame++;
      } else {
        blinkH = EYE_H;
        drawEyes(0, 0);
        display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
        int len = strlen(config.botName) * 6;
        display.setCursor((128 - len) / 2, 52);
        display.printf("Hi! I'm %s", config.botName);
        animPlay(ANIM_IDLE);
      }
      break;
    }

    default:
      drawEyes(0, 0);
      drawNameTag();
      drawTimerBorder();
      break;
  }

  // Timer border always on face screen
  drawTimerBorder();
  display.display();
}

// ──────────────────────────────────────────────────────────────
//  Buzzer helpers
// ──────────────────────────────────────────────────────────────

static const int BUZZER_CHANNEL = 0;
static bool buzzerAttached = false;

// Compatibility wrapper for old/new ESP32 LEDC APIs
void buzzerBegin(int freq) {
  freq = max(1, freq);

#if defined(ESP32)
  // Newer ESP32 core
  #if __has_include(<esp32-hal-ledc.h>)
    if (!buzzerAttached) {
      ledcAttach(BUZZER_PIN, freq, 8);
      buzzerAttached = true;
    } else {
      ledcChangeFrequency(BUZZER_PIN, freq, 8);
    }
  #else
    // Older ESP32 core
    ledcSetup(BUZZER_CHANNEL, freq, 8);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    buzzerAttached = true;
  #endif
#endif
}

void buzzerWriteDuty(int duty) {
#if defined(ESP32)
  #if __has_include(<esp32-hal-ledc.h>)
    ledcWrite(BUZZER_PIN, duty);
  #else
    ledcWrite(BUZZER_CHANNEL, duty);
  #endif
#endif
}

void buzzerWriteFreq(int freq, int duty) {
  buzzerBegin(freq);
  buzzerWriteDuty(duty);
}

void buzzerStop() {
#if defined(ESP32)
  #if __has_include(<esp32-hal-ledc.h>)
    ledcWrite(BUZZER_PIN, 0);
  #else
    ledcWrite(BUZZER_CHANNEL, 0);
  #endif
#endif
}

void buzzerTone(int freq, int durationMs) {
  if (!config.buzzerEnabled) return;

  int duty = map(config.buzzerVolume, 1, 10, 20, 200);
  buzzerWriteFreq(freq, duty);

  unsigned long s = millis();
  while (millis() - s < (unsigned long)durationMs) {
    webServerTick();
  }

  buzzerStop();
}

// Melody 0=startup  1=happy  2=timer set  3=sleep  4=alarm
// 5=scared  6=guitar strum  7=whistle  8=coffee sip  9=magnify
void buzzerMelody(int id) {
  if (!config.buzzerEnabled) return;

  struct Note { int freq; int dur; };
  const Note startup[]  = {{523,70},{659,70},{784,100},{1047,180},{0,40},{1047,70}};
  const Note happy[]    = {{784,70},{880,70},{1047,140}};
  const Note timerSet[] = {{880,55},{1047,90}};
  const Note sleep[]    = {{523,180},{440,180},{349,260}};
  const Note alarm[]    = {{1047,90},{0,40},{1047,90},{0,40},{1047,260}};
  const Note scared[]   = {{200,55},{160,55},{120,90},{0,25},{160,70}};
  const Note guitar[]   = {{330,60},{392,60},{494,60},{523,120},{0,30},{523,80}};
  const Note whistle[]  = {{1175,80},{1319,80},{1568,160}};
  const Note coffee[]   = {{523,120},{0,60},{494,120}};
  const Note magnify[]  = {{784,60},{880,90},{784,60}};
  const Note fly[]      = {{880,40},{0,20},{880,40},{0,20},{1047,80}};
  const Note typing[]   = {{1200,30},{0,20},{1100,30},{0,20},{1300,50}};

  const Note* notes = nullptr;
  int len = 0;
  switch (id) {
    case 0:  notes = startup;  len = 6; break;
    case 1:  notes = happy;    len = 3; break;
    case 2:  notes = timerSet; len = 2; break;
    case 3:  notes = sleep;    len = 3; break;
    case 4:  notes = alarm;    len = 5; break;
    case 5:  notes = scared;   len = 5; break;
    case 6:  notes = guitar;   len = 6; break;
    case 7:  notes = whistle;  len = 3; break;
    case 8:  notes = coffee;   len = 3; break;
    case 9:  notes = magnify;  len = 3; break;
    case 10: notes = fly;      len = 5; break;
    case 11: notes = typing;   len = 5; break;
  }
  if (!notes) return;

  int duty = map(config.buzzerVolume, 1, 10, 20, 200);

  for (int i = 0; i < len; i++) {
    if (notes[i].freq == 0) {
      buzzerStop();
    } else {
      buzzerWriteFreq(notes[i].freq, duty);
    }

    unsigned long s = millis();
    while (millis() - s < (unsigned long)notes[i].dur) {
      webServerTick();
    }
  }

  buzzerStop();
}
