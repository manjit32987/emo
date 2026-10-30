#include "BleController.h"
#include "DisplayController.h"
#include "AudioController.h"
#include "MotionController.h"
#include "TouchController.h"
#include "AlarmController.h"

BleController::BleController(DisplayController& display,
                             AudioController& audio,
                             MotionController& motion,
                             TouchController& touch,
                             AlarmController& alarm)
  : _display(display),
    _audio(audio),
    _motion(motion),
    _touch(touch),
    _alarm(alarm),
    _pServer(nullptr),
    _pTxCharacteristic(nullptr),
    _pRxCharacteristic(nullptr),
    _deviceConnected(false),
    _oldDeviceConnected(false),
    _autoMode(true),
    _lastTelemetryTime(0) {}

bool BleController::begin() {
  Serial.println("[BLE] Initializing Bluetooth Low Energy (BLE 5.0)...");
  BLEDevice::init("EMO-Companion");

  // Create BLE Server
  _pServer = BLEDevice::createServer();
  _pServer->setCallbacks(this);

  // Create BLE Service
  BLEService* pService = _pServer->createService(SERVICE_UUID);

  // TX Characteristic (ESP32 sends telemetry to Mobile App)
  _pTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  _pTxCharacteristic->addDescriptor(new BLE2902());

  // RX Characteristic (Mobile App sends commands to ESP32)
  _pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  _pRxCharacteristic->setCallbacks(this);

  // Start Service & Advertising
  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // Functions that help with iPhone connections
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Advertising started as 'EMO-Companion'. Ready for mobile app pairing.");
  return true;
}

void BleController::onConnect(BLEServer* pServer) {
  _deviceConnected = true;
  Serial.println("[BLE] Mobile app connected via Bluetooth!");
  _audio.play(SOUND_BOOT); // Happy chime on app connect
}

void BleController::onDisconnect(BLEServer* pServer) {
  _deviceConnected = false;
  Serial.println("[BLE] Mobile app disconnected.");
}

void BleController::update() {
  // Handle disconnection and re-advertising
  if (!_deviceConnected && _oldDeviceConnected) {
    delay(500);
    _pServer->startAdvertising();
    Serial.println("[BLE] Restarted advertising for reconnect.");
    _oldDeviceConnected = _deviceConnected;
  }
  if (_deviceConnected && !_oldDeviceConnected) {
    _oldDeviceConnected = _deviceConnected;
  }

  // Periodic Telemetry Stream to Mobile App (every 1.5 seconds)
  if (_deviceConnected && (millis() - _lastTelemetryTime > 1500)) {
    _lastTelemetryTime = millis();
    sendTelemetry();
  }
}

void BleController::sendTelemetry() {
  if (!_deviceConnected || !_pTxCharacteristic) return;

  String json = "{";
  json += "\"mood\":" + String((int)_display.getMood()) + ",";
  json += "\"style\":" + String((int)_display.getStyle()) + ",";
  json += "\"auto\":" + String(_autoMode ? "true" : "false") + ",";
  json += "\"time\":\"" + _alarm.getFormattedTime() + "\",";
  json += "\"alarmRing\":" + String(_alarm.isAlarmRinging() ? "true" : "false") + ",";
  json += "\"tiltX\":" + String(_motion.getTiltX()) + ",";
  json += "\"tiltY\":" + String(_motion.getTiltY()) + ",";
  json += "\"head\":" + String(_touch.isHeadHeld() ? "true" : "false") + ",";
  json += "\"cheek\":" + String(_touch.isCheekHeld() ? "true" : "false");
  json += "}\n";

  _pTxCharacteristic->setValue((uint8_t*)json.c_str(), json.length());
  _pTxCharacteristic->notify();
}

void BleController::onWrite(BLECharacteristic* pCharacteristic) {
  String rxValue = pCharacteristic->getValue();
  if (rxValue.length() > 0) {
    Serial.printf("[BLE] Command received: %s\n", rxValue.c_str());
    parseCommand(rxValue);
  }
}

// Simple fast key-value parser for commands sent from mobile app
void BleController::parseCommand(const String& cmdStr) {
  // Format examples:
  // {"cmd":"mood","val":"HAPPY"}
  // {"cmd":"vol","val":25}
  // {"cmd":"sound","val":2}
  // {"cmd":"style","val":1}
  // {"cmd":"alarm_act","val":"dismiss"}
  // {"cmd":"time","h":14,"m":30,"s":0}
  // {"cmd":"alarm","idx":0,"h":7,"m":30,"en":1,"label":"Wake"}
  // {"cmd":"mode","val":"auto"}

  if (cmdStr.indexOf("\"cmd\":\"mood\"") >= 0) {
    if (cmdStr.indexOf("\"NORMAL\"") >= 0)    _display.setMood(MOOD_NORMAL);
    else if (cmdStr.indexOf("\"HAPPY\"") >= 0)     { _display.setMood(MOOD_HAPPY); _audio.play(SOUND_HAPPY); }
    else if (cmdStr.indexOf("\"LOVE\"") >= 0)      { _display.setMood(MOOD_LOVE); _audio.play(SOUND_LOVE); }
    else if (cmdStr.indexOf("\"DIZZY\"") >= 0)     { _display.setMood(MOOD_DIZZY); _audio.play(SOUND_DIZZY); }
    else if (cmdStr.indexOf("\"SURPRISED\"") >= 0) { _display.setMood(MOOD_SURPRISED); _audio.play(SOUND_SURPRISED); }
    else if (cmdStr.indexOf("\"SAD\"") >= 0)       { _display.setMood(MOOD_SAD); _audio.play(SOUND_SAD); }
    else if (cmdStr.indexOf("\"ANGRY\"") >= 0)     { _display.setMood(MOOD_ANGRY); }
    else if (cmdStr.indexOf("\"SLEEPY\"") >= 0)    { _display.setMood(MOOD_SLEEPY); _audio.play(SOUND_SLEEP); }
  }
  else if (cmdStr.indexOf("\"cmd\":\"vol\"") >= 0) {
    int pos = cmdStr.indexOf("\"val\":");
    if (pos >= 0) {
      int vol = cmdStr.substring(pos + 6).toInt();
      _audio.setVolume(vol);
    }
  }
  else if (cmdStr.indexOf("\"cmd\":\"sound\"") >= 0) {
    int pos = cmdStr.indexOf("\"val\":");
    if (pos >= 0) {
      int track = cmdStr.substring(pos + 6).toInt();
      _audio.play((SoundTrack)track);
    }
  }
  else if (cmdStr.indexOf("\"cmd\":\"style\"") >= 0) {
    int pos = cmdStr.indexOf("\"val\":");
    if (pos >= 0) {
      int style = cmdStr.substring(pos + 6).toInt();
      _display.setStyle((EyeStyle)style);
    }
  }
  else if (cmdStr.indexOf("\"cmd\":\"alarm_act\"") >= 0) {
    if (cmdStr.indexOf("\"dismiss\"") >= 0) {
      _alarm.dismissAlarm();
      _display.setMood(MOOD_NORMAL);
    } else if (cmdStr.indexOf("\"snooze\"") >= 0) {
      _alarm.snoozeAlarm(5);
      _display.setMood(MOOD_NORMAL);
    } else if (cmdStr.indexOf("\"trigger\"") >= 0) {
      _alarm.triggerAlarm();
      _display.setMood(MOOD_ALARM);
      _audio.play(SOUND_ALARM);
    }
  }
  else if (cmdStr.indexOf("\"cmd\":\"time\"") >= 0) {
    int hPos = cmdStr.indexOf("\"h\":");
    int mPos = cmdStr.indexOf("\"m\":");
    int sPos = cmdStr.indexOf("\"s\":");
    if (hPos >= 0 && mPos >= 0) {
      int h = cmdStr.substring(hPos + 4).toInt();
      int m = cmdStr.substring(mPos + 4).toInt();
      int s = (sPos >= 0) ? cmdStr.substring(sPos + 4).toInt() : 0;
      _alarm.setTime(h, m, s);
    }
  }
  else if (cmdStr.indexOf("\"cmd\":\"alarm\"") >= 0) {
    int idxPos = cmdStr.indexOf("\"idx\":");
    int hPos   = cmdStr.indexOf("\"h\":");
    int mPos   = cmdStr.indexOf("\"m\":");
    int enPos  = cmdStr.indexOf("\"en\":");
    if (idxPos >= 0 && hPos >= 0 && mPos >= 0 && enPos >= 0) {
      int idx = cmdStr.substring(idxPos + 6).toInt();
      int h   = cmdStr.substring(hPos + 4).toInt();
      int m   = cmdStr.substring(mPos + 4).toInt();
      bool en = (cmdStr.substring(enPos + 5).toInt() == 1);
      _alarm.setAlarm(idx, h, m, en);
    }
  }
  else if (cmdStr.indexOf("\"cmd\":\"mode\"") >= 0) {
    if (cmdStr.indexOf("\"auto\"") >= 0) _autoMode = true;
    else if (cmdStr.indexOf("\"manual\"") >= 0) _autoMode = false;
  }
  else if (cmdStr.indexOf("\"cmd\":\"gaze\"") >= 0) {
    int xPos = cmdStr.indexOf("\"x\":");
    int yPos = cmdStr.indexOf("\"y\":");
    if (xPos >= 0 && yPos >= 0) {
      int x = cmdStr.substring(xPos + 4).toInt();
      int y = cmdStr.substring(yPos + 4).toInt();
      _display.setGaze(x, y);
    }
  }

  // Send immediate telemetry back confirming update
  sendTelemetry();
}

bool BleController::isConnected() const {
  return _deviceConnected;
}

bool BleController::isAutoMode() const {
  return _autoMode;
}

void BleController::setAutoMode(bool autoMode) {
  _autoMode = autoMode;
}

