#pragma once
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include "Config.h"

class AudioController {
public:
  AudioController();
  bool begin();
  void play(SoundTrack track);
  void setVolume(uint8_t volume); // 0 to 30
  bool isReady() const;

private:
  HardwareSerial _serial;
  DFRobotDFPlayerMini _dfplayer;
  bool _ready;
};

