#pragma once
#include <Arduino.h>

struct AlarmEntry {
  uint8_t hour    = 7;
  uint8_t minute  = 0;
  bool enabled    = false;
  char label[16]  = "Wake Up";
};

class AlarmController {
public:
  AlarmController();
  void begin();
  void update();

  void setTime(int hours, int minutes, int seconds);
  int getHours() const;
  int getMinutes() const;
  int getSeconds() const;
  String getFormattedTime() const;

  void setAlarm(int index, int hour, int minute, bool enabled, const char* label = "Alarm");
  AlarmEntry getAlarm(int index) const;
  int getAlarmCount() const;

  // Changeable Alarm Tone Track (e.g. 1 for 001.mp3, 2 for 002.mp3, etc.)
  void setAlarmTone(uint8_t trackNumber);
  uint8_t getAlarmTone() const;

  bool isAlarmRinging() const;
  void triggerAlarm();
  void dismissAlarm();
  void snoozeAlarm(int snoozeMinutes = 5);

private:
  int _hours;
  int _minutes;
  int _seconds;
  unsigned long _lastSecondTick;

  static const int MAX_ALARMS = 4;
  AlarmEntry _alarms[MAX_ALARMS];
  uint8_t _alarmTone; // Default: 1 (001.mp3)
  bool _isRinging;
  int _activeAlarmIndex;
  int _lastTriggeredMinute;
};
