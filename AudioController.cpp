#include "AudioController.h"

AudioController::AudioController() : _serial(1), _ready(false) {}

bool AudioController::begin() {
  _serial.begin(9600, SERIAL_8N1, PIN_MP3_RX, PIN_MP3_TX);
  delay(200);

  if (!_dfplayer.begin(_serial)) {
    Serial.println("[Audio] DFPlayer Mini initialization failed or timed out.");
    _ready = false;
    return false;
  }

  _ready = true;
  _dfplayer.volume(22); // Default friendly volume (0-30)
  Serial.println("[Audio] DFPlayer Mini initialized successfully.");
  return true;
}

void AudioController::play(SoundTrack track) {
  if (!_ready || track == SOUND_NONE) return;
  _dfplayer.play((int)track);
}

void AudioController::setVolume(uint8_t volume) {
  if (!_ready) return;
  _dfplayer.volume(constrain(volume, 0, 30));
}

bool AudioController::isReady() const {
  return _ready;
}

