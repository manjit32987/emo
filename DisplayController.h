#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Config.h"

#if defined(USE_SH1106)
  #include <Adafruit_SH110X.h>
#elif defined(USE_SSD1306)
  #include <Adafruit_SSD1306.h>
#endif

struct EyeParams {
  int width       = 36;
  int height      = 46;
  int radius      = 12;
  int spacing     = 8;
  int offsetX     = 0;  // Gaze offset X (-12 to 12)
  int offsetY     = 0;  // Gaze offset Y (-8 to 8)
  
  // Dynamic animated states
  float animHeight = 46.0f;
  float animWidth  = 36.0f;
  bool isBlinking  = false;
  float blinkProgress = 0.0f;
  unsigned long blinkStartTime = 0;
};

class DisplayController {
public:
  DisplayController();
  bool begin();
  void showBootMessage(const char* message);
  void showClock(int hours, int minutes, bool pm);

  void setMood(Mood mood);
  Mood getMood() const;
  
  void setStyle(EyeStyle style);
  EyeStyle getStyle() const;

  void setGaze(int offsetX, int offsetY);
  void triggerBlink();
  void updateAnimations();
  void render();

private:
#if defined(USE_SH1106)
  Adafruit_SH1106G _display;
#elif defined(USE_SSD1306)
  Adafruit_SSD1306 _display;
#endif

  Mood _mood;
  EyeStyle _style;
  EyeParams _eye;

  unsigned long _nextBlinkTime;
  unsigned long _nextSaccadeTime;

  // Drawing helpers
  void drawEyeShape(int cx, int cy, int w, int h, int r);
  void drawHighlights(int cx, int cy, int w, int h);
  void drawHeart(int cx, int cy, int size);
  void drawSwirl(int cx, int cy, int radius, float angle);
  void drawBell(int cx, int cy, int size);

  // Expression renderers
  void renderNormal(int leftX, int rightX, int eyeY);
  void renderHappy(int leftX, int rightX, int eyeY);
  void renderLove(int leftX, int rightX, int eyeY);
  void renderDizzy(int leftX, int rightX, int eyeY, int centerX);
  void renderSurprised(int leftX, int rightX, int eyeY, int centerX, int centerY);
  void renderSad(int leftX, int rightX, int eyeY);
  void renderAngry(int leftX, int rightX, int eyeY);
  void renderSleepy(int leftX, int rightX, int eyeY, int centerX, int centerY);
  void renderAlarm(int leftX, int rightX, int eyeY, int centerX, int centerY);
};
