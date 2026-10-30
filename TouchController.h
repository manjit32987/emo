#pragma once
#include <Arduino.h>
#include "Config.h"

class TouchController {
public:
  TouchController();
  void begin();
  void update();

  bool wasHeadPressed();
  bool wasCheekPressed();
  bool isHeadHeld() const;
  bool isCheekHeld() const;

private:
  bool _prevHeadState;
  bool _prevCheekState;
  bool _headPressedEvent;
  bool _cheekPressedEvent;
  bool _headCurrentState;
  bool _cheekCurrentState;

  unsigned long _lastDebounceHead;
  unsigned long _lastDebounceCheek;
};

