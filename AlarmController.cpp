#include "AlarmController.h"

AlarmController::AlarmController()
  : _hours(12),
    _minutes(0),
    _seconds(0),
    _lastSecondTick(0),
    _alarmTone(1), // Default to Track 1 (001.mp3 on MicroSD)
    _isRinging(false),
    _activeAlarmIndex(-1),
    _lastTriggeredMinute(-1) {
  _alarms[0] = { 7, 0, false, "Morning" };
  _alarms[1] = { 8, 30, false, "Work" };
  _alarms[2] = { 13, 0, false, "Lunch" };
  _alarms[3] = { 22, 0, false, "Bedtime" };
}

void AlarmController::begin() {
  _lastSecondTick = millis();
}

void AlarmController::update() {
  unsigned long now = millis();
  if (now - _lastSecondTick >= 1000) {
    _lastSecondTick += 1000;
    _seconds++;
    if (_seconds >= 60) {
      _seconds = 0;
      _minutes++;
      if (_minutes >= 60) {
        _minutes = 0;
        _hours++;
        if (_hours >= 24) {
          _hours = 0;
        }
      }

      // Check alarms once per minute
      if (_lastTriggeredMinute != _minutes) {
        for (int i = 0; i < MAX_ALARMS; i++) {
          if (_alarms[i].enabled && _alarms[i].hour == _hours && _alarms[i].minute == _minutes) {
            _isRinging = true;
            _activeAlarmIndex = i;
            _lastTriggeredMinute = _minutes;
            Serial.printf("[Alarm] Alarm %d (%s) triggered at %02d:%02d with tone track %d!\n", 
                          i, _alarms[i].label, _hours, _minutes, _alarmTone);
            break;
          }
        }
      }
    }
  }
}

void AlarmController::setTime(int hours, int minutes, int seconds) {
  _hours = constrain(hours, 0, 23);
  _minutes = constrain(minutes, 0, 59);
  _seconds = constrain(seconds, 0, 59);
  _lastSecondTick = millis();
  Serial.printf("[Clock] Time synchronized to %02d:%02d:%02d\n", _hours, _minutes, _seconds);
}

int AlarmController::getHours() const { return _hours; }
int AlarmController::getMinutes() const { return _minutes; }
int AlarmController::getSeconds() const { return _seconds; }

String AlarmController::getFormattedTime() const {
  char buf[12];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", _hours, _minutes, _seconds);
  return String(buf);
}

void AlarmController::setAlarm(int index, int hour, int minute, bool enabled, const char* label) {
  if (index < 0 || index >= MAX_ALARMS) return;
  _alarms[index].hour = constrain(hour, 0, 23);
  _alarms[index].minute = constrain(minute, 0, 59);
  _alarms[index].enabled = enabled;
  if (label && strlen(label) > 0) {
    strncpy(_alarms[index].label, label, sizeof(_alarms[index].label) - 1);
  }
}

AlarmEntry AlarmController::getAlarm(int index) const {
  if (index < 0 || index >= MAX_ALARMS) return _alarms[0];
  return _alarms[index];
}

int AlarmController::getAlarmCount() const {
  return MAX_ALARMS;
}

void AlarmController::setAlarmTone(uint8_t trackNumber) {
  _alarmTone = constrain(trackNumber, 1, 99);
  Serial.printf("[Alarm] Alarm tone changed to track: %03d.mp3\n", _alarmTone);
}

uint8_t AlarmController::getAlarmTone() const {
  return _alarmTone;
}

bool AlarmController::isAlarmRinging() const {
  return _isRinging;
}

void AlarmController::triggerAlarm() {
  _isRinging = true;
}

void AlarmController::dismissAlarm() {
  _isRinging = false;
  _activeAlarmIndex = -1;
  Serial.println("[Alarm] Alarm dismissed.");
}

void AlarmController::snoozeAlarm(int snoozeMinutes) {
  _isRinging = false;
  _minutes = (_minutes + snoozeMinutes) % 60;
  Serial.printf("[Alarm] Alarm snoozed for %d minutes.\n", snoozeMinutes);
}
