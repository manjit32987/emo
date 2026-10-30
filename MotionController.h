#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "Config.h"

class MotionController {
public:
  MotionController();
  bool begin();
  void update();

  bool isShaken();
  bool isFreefall();
  bool hasMovement() const;
  int getTiltX() const;
  int getTiltY() const;
  bool isReady() const;

private:
  Adafruit_MPU6050 _mpu;
  bool _ready;

  float _prevAccelMag;
  unsigned long _lastShakeTime;

  bool _shakenEvent;
  bool _freefallEvent;
  bool _movementDetected;

  int _tiltX;
  int _tiltY;
};

