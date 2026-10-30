#include "MotionController.h"

MotionController::MotionController()
  : _ready(false),
    _prevAccelMag(9.8f),
    _lastShakeTime(0),
    _shakenEvent(false),
    _freefallEvent(false),
    _movementDetected(false),
    _tiltX(0),
    _tiltY(0) {}

bool MotionController::begin() {
  if (!_mpu.begin(MPU_I2C_ADDRESS, &Wire)) {
    Serial.println("[Motion] MPU6050 not found on 0x68!");
    _ready = false;
    return false;
  }

  _mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  _mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  _ready = true;

  Serial.println("[Motion] MPU6050 initialized successfully.");
  return true;
}

void MotionController::update() {
  _shakenEvent = false;
  _freefallEvent = false;
  _movementDetected = false;

  if (!_ready) return;

  sensors_event_t a, g, temp;
  _mpu.getEvent(&a, &g, &temp);

  // Compute total acceleration magnitude
  float accelMag = sqrt(a.acceleration.x * a.acceleration.x +
                        a.acceleration.y * a.acceleration.y +
                        a.acceleration.z * a.acceleration.z);

  float deltaAccel = abs(accelMag - _prevAccelMag);
  _prevAccelMag = accelMag;

  float gyroMag = sqrt(g.gyro.x * g.gyro.x + g.gyro.y * g.gyro.y + g.gyro.z * g.gyro.z);

  // 1. Shake Detection
  unsigned long now = millis();
  if ((deltaAccel > SHAKE_ACCEL_DELTA || gyroMag > SHAKE_GYRO_MAG) && (now - _lastShakeTime > 800)) {
    _lastShakeTime = now;
    _shakenEvent = true;
    _movementDetected = true;
    return;
  }

  // 2. Freefall / Sudden Drop Detection
  if (accelMag < FREEFALL_THRESHOLD) {
    _freefallEvent = true;
    _movementDetected = true;
    return;
  }

  // 3. Tilt Calculation (for eye tracking)
  _tiltX = constrain((int)(a.acceleration.x * 1.5f), -12, 12);
  _tiltY = constrain((int)(a.acceleration.y * 1.2f), -8, 8);

  // 4. Movement activity detection
  if (abs(a.acceleration.x) > 2.0f || abs(a.acceleration.y) > 2.0f || gyroMag > 0.6f) {
    _movementDetected = true;
  }
}

bool MotionController::isShaken() {
  return _shakenEvent;
}

bool MotionController::isFreefall() {
  return _freefallEvent;
}

bool MotionController::hasMovement() const {
  return _movementDetected;
}

int MotionController::getTiltX() const {
  return _tiltX;
}

int MotionController::getTiltY() const {
  return _tiltY;
}

bool MotionController::isReady() const {
  return _ready;
}

