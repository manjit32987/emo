/**
 * ============================================================================
 * Project: ESP32-C3 Desktop Companion (EMO)
 * ============================================================================
 * Features:
 *   - Bluetooth Low Energy (BLE 5.0) Client-Server Interface
 *     (The ESP32 does NOT host any web page; it connects natively to the
 *      installed mobile app via BLE Nordic UART Service)
 *   - High-Definition Procedural Living Eyes Engine (Squash & stretch blinks,
 *     eyelid curves, glassy dual highlights, parametric saccades & styles)
 *   - Alarm Manager with Smartphone Time Sync and Snooze/Dismiss actions
 *   - MPU6050 6-Axis Motion & Tilt Tracking (Shake, Drop/Lift, Gravity Gaze)
 *   - Dual Capacitive Touch Sensors (Head Petting & Cheek Poking)
 *   - MP3-TF-16P (DFPlayer Mini) Hardware UART Audio FX
 * ============================================================================
 */

#include <Wire.h>
#include "Config.h"
#include "DisplayController.h"
#include "AudioController.h"
#include "MotionController.h"
#include "TouchController.h"
#include "AlarmController.h"
#include "BleController.h"

// --- CONTROLLER SUBSYSTEMS ---
DisplayController displayCtrl;
AudioController   audioCtrl;
MotionController  motionCtrl;
TouchController   touchCtrl;
AlarmController   alarmCtrl;
BleController     bleCtrl(displayCtrl, audioCtrl, motionCtrl, touchCtrl, alarmCtrl);

// --- COMPANION STATE & TIMERS ---
unsigned long moodStartTime    = 0;
unsigned long moodDuration     = 2500;
unsigned long lastActivityTime = 0;
unsigned long lastAlarmBeep    = 0;

void triggerMood(Mood newMood, unsigned long durationMs = 2500, SoundTrack sound = SOUND_NONE) {
  displayCtrl.setMood(newMood);
  moodStartTime    = millis();
  moodDuration     = durationMs;
  lastActivityTime = millis();

  if (sound != SOUND_NONE) {
    audioCtrl.play(sound);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n==========================================");
  Serial.println("  ESP32-C3 Desktop Companion Initializing");
  Serial.println("==========================================");

  // 1. Shared I2C Bus setup on GPIO 8 (SDA) and GPIO 9 (SCL)
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(400000); // 400kHz fast mode for fluid animations

  // 2. Initialize Subsystems
  displayCtrl.begin();
  displayCtrl.showBootMessage("Starting EMO...");

  touchCtrl.begin();
  motionCtrl.begin();
  audioCtrl.begin();
  alarmCtrl.begin();

  // 3. Initialize BLE 5.0 (Connects to mobile app; no web server hosted)
  bleCtrl.begin();

  // 4. Companion Wakeup
  lastActivityTime = millis();
  triggerMood(MOOD_HAPPY, 1800, SOUND_BOOT);
}

void loop() {
  unsigned long now = millis();

  // --- 1. BLUETOOTH LOW ENERGY TICK ---
  bleCtrl.update();

  // --- 2. CLOCK & ALARM TICK ---
  alarmCtrl.update();

  // Check if Alarm is active
  if (alarmCtrl.isAlarmRinging()) {
    displayCtrl.setMood(MOOD_ALARM);

    // Ring alarm sound every 2.5 seconds
    if (now - lastAlarmBeep > 2500) {
      lastAlarmBeep = now;
      audioCtrl.play(SOUND_ALARM);
    }

    // Dismiss alarm by physical touch on head or cheek (or from mobile app)
    touchCtrl.update();
    if (touchCtrl.wasHeadPressed() || touchCtrl.wasCheekPressed()) {
      alarmCtrl.dismissAlarm();
      triggerMood(MOOD_HAPPY, 2000, SOUND_HAPPY);
    }

    displayCtrl.render();
    delay(20);
    return; // Prioritize alarm interaction
  }

  // --- 3. SENSOR POLLING ---
  touchCtrl.update();
  motionCtrl.update();

  // --- 4. AUTONOMOUS BEHAVIOR (When in Auto AI Mode) ---
  if (bleCtrl.isAutoMode()) {
    // Touch reactions
    if (touchCtrl.wasHeadPressed()) {
      triggerMood(MOOD_LOVE, 3500, SOUND_LOVE);       // Petting -> Heart eyes & purr
    } else if (touchCtrl.wasCheekPressed()) {
      triggerMood(MOOD_HAPPY, 2500, SOUND_HAPPY);     // Poke -> Smiling crescent cheeks
    }

    // Motion reactions
    if (motionCtrl.isShaken()) {
      triggerMood(MOOD_DIZZY, 3000, SOUND_DIZZY);     // Shaken -> Dizzy swirls
    } else if (motionCtrl.isFreefall()) {
      triggerMood(MOOD_SURPRISED, 2000, SOUND_SURPRISED); // Drop / Pick up -> Wide shock eyes
    }

    // Inactivity tracking
    if (motionCtrl.hasMovement() || touchCtrl.isHeadHeld() || touchCtrl.isCheekHeld()) {
      lastActivityTime = now;
    }

    // Return to Normal when mood timer expires
    if (displayCtrl.getMood() != MOOD_NORMAL && (now - moodStartTime > moodDuration)) {
      displayCtrl.setMood(MOOD_NORMAL);
      displayCtrl.setGaze(0, 0);
    }

    // Stillness sleep timer (30s)
    if (displayCtrl.getMood() == MOOD_NORMAL && (now - lastActivityTime > SLEEP_TIMEOUT_MS)) {
      triggerMood(MOOD_SLEEPY, 60000, SOUND_SLEEP);
    }

    // Eye gaze tilt tracking
    if (displayCtrl.getMood() == MOOD_NORMAL) {
      if (abs(motionCtrl.getTiltX()) > 2 || abs(motionCtrl.getTiltY()) > 2) {
        displayCtrl.setGaze(motionCtrl.getTiltX(), motionCtrl.getTiltY());
      }
    }
  }

  // --- 5. ANIMATIONS & RENDERING ---
  displayCtrl.updateAnimations();
  displayCtrl.render();

  delay(20); // ~50 FPS target
}
