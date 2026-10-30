#pragma once
#include <Arduino.h>

// ============================================================================
// 1. PIN DEFINITIONS (ESP32-C3 SuperMini)
// ============================================================================
#define PIN_I2C_SDA         8   // Shared I2C SDA
#define PIN_I2C_SCL         9   // Shared I2C SCL

#define PIN_MP3_RX          2   // ESP32 RX <- DFPlayer TX
#define PIN_MP3_TX          3   // ESP32 TX -> 1k resistor -> DFPlayer RX

#define PIN_TOUCH_HEAD      4   // Capacitive Touch 1 (Head / Pet)
#define PIN_TOUCH_CHEEK     10  // Capacitive Touch 2 (Cheek / Poke)

#define TOUCH_ACTIVE_STATE  HIGH // TTP223 default active HIGH

// ============================================================================
// 2. DISPLAY DRIVER CONFIGURATION
// ============================================================================
#define USE_SH1106
//#define USE_SSD1306

#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64
#define OLED_I2C_ADDRESS    0x3C
#define OLED_RESET_PIN      -1

// ============================================================================
// 3. MPU6050 CONFIGURATION
// ============================================================================
#define MPU_I2C_ADDRESS     0x68

// ============================================================================
// 4. WI-FI & MOBILE APP CONFIGURATION
// ============================================================================
#define WIFI_AP_SSID        "EMO-Companion"
#define WIFI_AP_PASS        "12345678"     // Minimum 8 chars, or "" for open network
#define WEB_SERVER_PORT     80

// ============================================================================
// 5. TIMING & SENSITIVITY THRESHOLDS
// ============================================================================
#define SLEEP_TIMEOUT_MS    30000   // Sleep after 30s of stillness
#define SHAKE_ACCEL_DELTA   11.0f   // Shake acceleration threshold (m/s^2)
#define SHAKE_GYRO_MAG      5.5f    // Shake rotation threshold (rad/s)
#define FREEFALL_THRESHOLD  3.0f    // Freefall / drop threshold (m/s^2)

// ============================================================================
// 6. EYE STYLES & EMOTIONS
// ============================================================================
enum EyeStyle {
  STYLE_EMO = 0,    // Iconic large rounded expressive eyes with highlights
  STYLE_ROUND,      // Cute oversized spherical anime eyes
  STYLE_CYBER,      // Sharp futuristic cyber visor eyes
  STYLE_CAT         // Slit-pupil playful cat eyes
};

enum Mood {
  MOOD_NORMAL = 0,
  MOOD_HAPPY,
  MOOD_LOVE,
  MOOD_DIZZY,
  MOOD_SURPRISED,
  MOOD_SAD,
  MOOD_ANGRY,
  MOOD_SLEEPY,
  MOOD_ALARM
};

// ============================================================================
// 7. AUDIO SOUND TRACKS (MicroSD card /01/xxx.mp3)
// ============================================================================
enum SoundTrack {
  SOUND_NONE      = 0,
  SOUND_BOOT      = 1,  // 001.mp3 - Startup chime
  SOUND_HAPPY     = 2,  // 002.mp3 - Happy chirp / giggle
  SOUND_LOVE      = 3,  // 003.mp3 - Love purr / kiss
  SOUND_DIZZY     = 4,  // 004.mp3 - Dizzy wobble
  SOUND_SURPRISED = 5,  // 005.mp3 - Gasp / shock
  SOUND_SLEEP     = 6,  // 006.mp3 - Yawn / snore
  SOUND_POKE      = 7,  // 007.mp3 - Poke response
  SOUND_ALARM     = 8,  // 008.mp3 - Ringing alarm chime
  SOUND_SAD       = 9   // 009.mp3 - Sad whimper
};
