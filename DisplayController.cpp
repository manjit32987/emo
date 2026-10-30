#include "DisplayController.h"

#if defined(USE_SH1106)
DisplayController::DisplayController()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN),
    _mood(MOOD_NORMAL),
    _style(STYLE_EMO),
    _nextBlinkTime(2000),
    _nextSaccadeTime(3000) {}
#elif defined(USE_SSD1306)
DisplayController::DisplayController()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN),
    _mood(MOOD_NORMAL),
    _style(STYLE_EMO),
    _nextBlinkTime(2000),
    _nextSaccadeTime(3000) {}
#endif

bool DisplayController::begin() {
#if defined(USE_SH1106)
  if (!_display.begin(OLED_I2C_ADDRESS, true)) {
    Serial.println("[Display] SH1106 OLED initialization failed!");
    return false;
  }
#elif defined(USE_SSD1306)
  if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println("[Display] SSD1306 OLED initialization failed!");
    return false;
  }
#endif

  _display.clearDisplay();
  _display.display();
  Serial.println("[Display] OLED initialized successfully with High-Definition Eye Engine.");
  return true;
}

void DisplayController::showBootMessage(const char* message) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(1);
  _display.setCursor(22, 28);
  _display.print(message);
  _display.display();
}

void DisplayController::showClock(int hours, int minutes, bool pm) {
  _display.clearDisplay();
  _display.setTextSize(2);
  _display.setTextColor(1);
  _display.setCursor(28, 22);

  if (hours < 10) _display.print("0");
  _display.print(hours);
  _display.print(":");
  if (minutes < 10) _display.print("0");
  _display.print(minutes);

  _display.setTextSize(1);
  _display.setCursor(92, 28);
  _display.print(pm ? "PM" : "AM");
  _display.display();
}

void DisplayController::setMood(Mood mood) {
  _mood = mood;
  if (_mood != MOOD_NORMAL) {
    _eye.isBlinking = false;
  }
}

Mood DisplayController::getMood() const {
  return _mood;
}

void DisplayController::setStyle(EyeStyle style) {
  _style = style;
}

EyeStyle DisplayController::getStyle() const {
  return _style;
}

void DisplayController::setGaze(int offsetX, int offsetY) {
  _eye.offsetX = constrain(offsetX, -12, 12);
  _eye.offsetY = constrain(offsetY, -8, 8);
}

void DisplayController::triggerBlink() {
  if (!_eye.isBlinking && _mood == MOOD_NORMAL) {
    _eye.isBlinking = true;
    _eye.blinkStartTime = millis();
    _eye.blinkProgress = 0.0f;
  }
}

void DisplayController::updateAnimations() {
  unsigned long now = millis();

  // 1. Organic Blink State Machine with Squash & Stretch
  if (_mood == MOOD_NORMAL) {
    if (!_eye.isBlinking && now > _nextBlinkTime) {
      triggerBlink();
    } else if (_eye.isBlinking) {
      unsigned long elapsed = now - _eye.blinkStartTime;
      const unsigned long totalBlinkMs = 180;

      if (elapsed >= totalBlinkMs) {
        _eye.isBlinking = false;
        _eye.animHeight = (float)_eye.height;
        _eye.animWidth = (float)_eye.width;
        // Natural random interval for next blink (2 to 6 seconds)
        _nextBlinkTime = now + random(2200, 6000);
      } else {
        float t = (float)elapsed / (float)totalBlinkMs; // 0.0 to 1.0
        // Parabolic easing for eyelid drop and recovery
        float closure = 4.0f * t * (1.0f - t); // peaks at 1.0 at t=0.5
        _eye.animHeight = _eye.height * (1.0f - closure * 0.90f);
        if (_eye.animHeight < 4.0f) _eye.animHeight = 4.0f;
        // Squash & stretch: widen slightly as height compresses
        _eye.animWidth = _eye.width + (closure * 4.0f);
      }
    }
  } else {
    _eye.animHeight = (float)_eye.height;
    _eye.animWidth  = (float)_eye.width;
  }

  // 2. Natural Saccadic Glance Updates
  if (_mood == MOOD_NORMAL && now > _nextSaccadeTime) {
    _eye.offsetX = random(-8, 9);
    _eye.offsetY = random(-4, 5);
    _nextSaccadeTime = now + random(2500, 5500);
  }
}

// ============================================================================
// DRAWING PRIMITIVES & STYLES
// ============================================================================

void DisplayController::drawEyeShape(int cx, int cy, int w, int h, int r) {
  int x = cx - w / 2;
  int y = cy - h / 2;

  switch (_style) {
    case STYLE_ROUND: {
      int rad = min(w, h) / 2;
      _display.fillCircle(cx, cy, rad, 1);
      break;
    }
    case STYLE_CYBER: {
      // Hexagonal / cut-corner tech eyes
      _display.fillRoundRect(x, y, w, h, 3, 1);
      _display.drawFastHLine(x + 4, y, w - 8, 0);
      _display.drawFastHLine(x + 4, y + h - 1, w - 8, 0);
      break;
    }
    case STYLE_CAT: {
      // Curved soft frame with vertical slit pupil
      _display.fillRoundRect(x, y, w, h, r, 1);
      _display.fillRoundRect(cx - 3, cy - h / 2 + 4, 6, h - 8, 3, 0);
      break;
    }
    case STYLE_EMO:
    default: {
      // Iconic EMO large organic rounded rectangles
      _display.fillRoundRect(x, y, w, h, r, 1);
      break;
    }
  }
}

void DisplayController::drawHighlights(int cx, int cy, int w, int h) {
  if (_style == STYLE_CAT) return; // Cat eyes have slit pupils

  // Primary large glossy specular glint (Upper Left, offset by gaze)
  int h1X = cx - (w / 4) + (_eye.offsetX / 4);
  int h1Y = cy - (h / 4) + (_eye.offsetY / 4);
  _display.fillCircle(h1X, h1Y, 4, 0);

  // Secondary subtle bottom-right reflection for glass depth
  int h2X = cx + (w / 4) + (_eye.offsetX / 4);
  int h2Y = cy + (h / 5) + (_eye.offsetY / 4);
  _display.fillCircle(h2X, h2Y, 2, 0);
}

void DisplayController::drawHeart(int cx, int cy, int size) {
  _display.fillCircle(cx - size / 2, cy - size / 4, size / 2, 1);
  _display.fillCircle(cx + size / 2, cy - size / 4, size / 2, 1);
  _display.fillTriangle(cx - size, cy - size / 8, cx + size, cy - size / 8, cx, cy + size, 1);
}

void DisplayController::drawSwirl(int cx, int cy, int radius, float angle) {
  for (int r = 3; r <= radius; r += 3) {
    float a = angle + (r * 0.5f);
    int px = cx + (int)(cos(a) * r);
    int py = cy + (int)(sin(a) * r);
    _display.drawCircle(px, py, 2, 1);
  }
}

void DisplayController::drawBell(int cx, int cy, int size) {
  _display.fillCircle(cx, cy - 2, size / 2, 1);
  _display.fillRoundRect(cx - size / 2, cy - 2, size, size / 2, 2, 1);
  _display.fillCircle(cx, cy + size / 2 + 1, 2, 1);
}

// ============================================================================
// MOOD RENDERERS
// ============================================================================

void DisplayController::renderNormal(int leftX, int rightX, int eyeY) {
  int w = (int)_eye.animWidth;
  int h = (int)_eye.animHeight;
  int r = _eye.radius;

  if (_eye.isBlinking && h <= 6) {
    // Smooth closed eyelid line with soft rounded tips
    _display.fillRoundRect(leftX - w / 2, eyeY - 2, w, 5, 2, 1);
    _display.fillRoundRect(rightX - w / 2, eyeY - 2, w, 5, 2, 1);
  } else {
    drawEyeShape(leftX, eyeY, w, h, r);
    drawEyeShape(rightX, eyeY, w, h, r);
    drawHighlights(leftX, eyeY, w, h);
    drawHighlights(rightX, eyeY, w, h);
  }
}

void DisplayController::renderHappy(int leftX, int rightX, int eyeY) {
  // Smiling thick curved crescents (^ ^)
  int w = _eye.width + 6;
  int h = _eye.height / 2 + 2;

  // Outer smiling arc
  _display.fillRoundRect(leftX - w / 2, eyeY - h / 2, w, h, 8, 1);
  _display.fillRoundRect(leftX - w / 2 + 3, eyeY - h / 2 + 6, w - 6, h, 8, 0);

  _display.fillRoundRect(rightX - w / 2, eyeY - h / 2, w, h, 8, 1);
  _display.fillRoundRect(rightX - w / 2 + 3, eyeY - h / 2 + 6, w - 6, h, 8, 0);

  // Rosy blushing cheeks
  _display.fillCircle(leftX - 19, eyeY + 16, 3, 1);
  _display.fillCircle(rightX + 19, eyeY + 16, 3, 1);
}

void DisplayController::renderLove(int leftX, int rightX, int eyeY) {
  // Beating Heart Eyes <3 <3
  static float pulse = 0.0f;
  pulse += 0.25f;
  int pulseSize = 12 + (int)(sin(pulse) * 3);

  drawHeart(leftX, eyeY, pulseSize);
  drawHeart(rightX, eyeY, pulseSize);

  // Sparkle glints
  _display.drawPixel(leftX - pulseSize / 2, eyeY - pulseSize / 2, 0);
  _display.drawPixel(rightX - pulseSize / 2, eyeY - pulseSize / 2, 0);
}

void DisplayController::renderDizzy(int leftX, int rightX, int eyeY, int centerX) {
  static float swirlAngle = 0.0f;
  swirlAngle += 0.4f;

  drawSwirl(leftX, eyeY, 15, swirlAngle);
  drawSwirl(rightX, eyeY, 15, -swirlAngle);

  // Wobbly confused mouth
  int mouthY = eyeY + 22;
  for (int x = -10; x <= 10; x++) {
    int yOff = (int)(sin((x + millis() / 80) * 0.6f) * 2.5f);
    _display.drawPixel(centerX + x, mouthY + yOff, 1);
  }
}

void DisplayController::renderSurprised(int leftX, int rightX, int eyeY, int centerX, int centerY) {
  int radius = 22;
  _display.fillCircle(leftX, eyeY, radius, 1);
  _display.fillCircle(rightX, eyeY, radius, 1);

  // Tiny shocked pupils
  _display.fillCircle(leftX + _eye.offsetX / 2, eyeY + _eye.offsetY / 2, 5, 0);
  _display.fillCircle(rightX + _eye.offsetX / 2, eyeY + _eye.offsetY / 2, 5, 0);

  // Dropped open circular mouth
  _display.drawCircle(centerX, centerY + 22, 5, 1);
}

void DisplayController::renderSad(int leftX, int rightX, int eyeY) {
  // Full eyes
  drawEyeShape(leftX, eyeY, _eye.width, _eye.height, _eye.radius);
  drawEyeShape(rightX, eyeY, _eye.width, _eye.height, _eye.radius);
  drawHighlights(leftX, eyeY, _eye.width, _eye.height);
  drawHighlights(rightX, eyeY, _eye.width, _eye.height);

  // Drooping top-outer triangular eyelid cut
  _display.fillTriangle(leftX - _eye.width / 2, eyeY - _eye.height / 2,
                        leftX - _eye.width / 2, eyeY - 2,
                        leftX + _eye.width / 2, eyeY - _eye.height / 2, 0);

  _display.fillTriangle(rightX + _eye.width / 2, eyeY - _eye.height / 2,
                        rightX + _eye.width / 2, eyeY - 2,
                        rightX - _eye.width / 2, eyeY - _eye.height / 2, 0);

  // Glistening tear
  _display.fillCircle(leftX - 10, eyeY + 16, 2, 1);
}

void DisplayController::renderAngry(int leftX, int rightX, int eyeY) {
  drawEyeShape(leftX, eyeY, _eye.width, _eye.height, _eye.radius);
  drawEyeShape(rightX, eyeY, _eye.width, _eye.height, _eye.radius);
  drawHighlights(leftX, eyeY, _eye.width, _eye.height);
  drawHighlights(rightX, eyeY, _eye.width, _eye.height);

  // Fierce inward-slanting brow cut
  _display.fillTriangle(leftX - _eye.width / 2, eyeY - _eye.height / 2,
                        leftX + _eye.width / 2, eyeY - _eye.height / 2,
                        leftX + _eye.width / 2, eyeY + 4, 0);

  _display.fillTriangle(rightX - _eye.width / 2, eyeY - _eye.height / 2,
                        rightX + _eye.width / 2, eyeY - _eye.height / 2,
                        rightX - _eye.width / 2, eyeY + 4, 0);
}

void DisplayController::renderSleepy(int leftX, int rightX, int eyeY, int centerX, int centerY) {
  // Relaxed closed lids
  _display.fillRoundRect(leftX - _eye.width / 2, eyeY + 4, _eye.width, 5, 2, 1);
  _display.fillRoundRect(rightX - _eye.width / 2, eyeY + 4, _eye.width, 5, 2, 1);

  // Animated floating "Z z z"
  int zOffset = (millis() / 350) % 22;
  _display.setTextSize(1);
  _display.setTextColor(1);
  _display.setCursor(centerX + 28, centerY - 4 - zOffset);
  _display.print("z");
  _display.setCursor(centerX + 38, centerY - 14 - zOffset);
  _display.print("Z");
}

void DisplayController::renderAlarm(int leftX, int rightX, int eyeY, int centerX, int centerY) {
  static bool flash = false;
  flash = ((millis() / 200) % 2 == 0);

  if (flash) {
    drawBell(leftX, eyeY, 20);
    drawBell(rightX, eyeY, 20);
    _display.setTextSize(2);
    _display.setTextColor(1);
    _display.setCursor(centerX - 6, centerY + 14);
    _display.print("!");
  } else {
    drawEyeShape(leftX, eyeY, _eye.width, _eye.height, _eye.radius);
    drawEyeShape(rightX, eyeY, _eye.width, _eye.height, _eye.radius);
    _display.setTextSize(2);
    _display.setTextColor(0);
    _display.setCursor(leftX - 6, eyeY - 8);
    _display.print("!");
    _display.setCursor(rightX - 6, eyeY - 8);
    _display.print("!");
  }
}

// ============================================================================
// MAIN RENDER LOOP
// ============================================================================

void DisplayController::render() {
  _display.clearDisplay();

  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  // Prominent eye centers (X = 42 and X = 86)
  int leftEyeX  = centerX - (_eye.spacing / 2) - (_eye.width / 2) + _eye.offsetX;
  int rightEyeX = centerX + (_eye.spacing / 2) + (_eye.width / 2) + _eye.offsetX;
  int eyeY      = centerY + _eye.offsetY;

  switch (_mood) {
    case MOOD_NORMAL:    renderNormal(leftEyeX, rightEyeX, eyeY); break;
    case MOOD_HAPPY:     renderHappy(leftEyeX, rightEyeX, eyeY); break;
    case MOOD_LOVE:      renderLove(leftEyeX, rightEyeX, eyeY); break;
    case MOOD_DIZZY:     renderDizzy(leftEyeX, rightEyeX, eyeY, centerX); break;
    case MOOD_SURPRISED: renderSurprised(leftEyeX, rightEyeX, eyeY, centerX, centerY); break;
    case MOOD_SAD:       renderSad(leftEyeX, rightEyeX, eyeY); break;
    case MOOD_ANGRY:     renderAngry(leftEyeX, rightEyeX, eyeY); break;
    case MOOD_SLEEPY:    renderSleepy(leftEyeX, rightEyeX, eyeY, centerX, centerY); break;
    case MOOD_ALARM:     renderAlarm(leftEyeX, rightEyeX, eyeY, centerX, centerY); break;
  }

  _display.display();
}
