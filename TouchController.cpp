#include "TouchController.h"

TouchController::TouchController()
  : _prevHeadState(false),
    _prevCheekState(false),
    _headPressedEvent(false),
    _cheekPressedEvent(false),
    _headCurrentState(false),
    _cheekCurrentState(false),
    _lastDebounceHead(0),
    _lastDebounceCheek(0) {}

void TouchController::begin() {
  pinMode(PIN_TOUCH_HEAD, INPUT);
  pinMode(PIN_TOUCH_CHEEK, INPUT);
  Serial.println("[Touch] Touch sensors initialized on GPIO 4 and 10.");
}

void TouchController::update() {
  _headPressedEvent = false;
  _cheekPressedEvent = false;

  bool rawHead = (digitalRead(PIN_TOUCH_HEAD) == TOUCH_ACTIVE_STATE);
  bool rawCheek = (digitalRead(PIN_TOUCH_CHEEK) == TOUCH_ACTIVE_STATE);

  unsigned long now = millis();

  // Head Debouncing & Edge Trigger
  if (rawHead != _prevHeadState && (now - _lastDebounceHead > 50)) {
    _lastDebounceHead = now;
    _prevHeadState = rawHead;
    if (rawHead) {
      _headPressedEvent = true;
    }
  }
  _headCurrentState = rawHead;

  // Cheek Debouncing & Edge Trigger
  if (rawCheek != _prevCheekState && (now - _lastDebounceCheek > 50)) {
    _lastDebounceCheek = now;
    _prevCheekState = rawCheek;
    if (rawCheek) {
      _cheekPressedEvent = true;
    }
  }
  _cheekCurrentState = rawCheek;
}

bool TouchController::wasHeadPressed() {
  return _headPressedEvent;
}

bool TouchController::wasCheekPressed() {
  return _cheekPressedEvent;
}

bool TouchController::isHeadHeld() const {
  return _headCurrentState;
}

bool TouchController::isCheekHeld() const {
  return _cheekCurrentState;
}

