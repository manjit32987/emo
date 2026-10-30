#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "Config.h"

// Forward declarations
class DisplayController;
class AudioController;
class MotionController;
class TouchController;
class AlarmController;

// Nordic UART Service (NUS) UUIDs - Standard for mobile BLE companion apps
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // App -> ESP32
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // ESP32 -> App

class BleController : public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
  BleController(DisplayController& display,
                AudioController& audio,
                MotionController& motion,
                TouchController& touch,
                AlarmController& alarm);

  bool begin();
  void update();
  void sendTelemetry();

  bool isConnected() const;
  bool isAutoMode() const;
  void setAutoMode(bool autoMode);

  // BLE Callbacks
  void onConnect(BLEServer* pServer) override;
  void onDisconnect(BLEServer* pServer) override;
  void onWrite(BLECharacteristic* pCharacteristic) override;

private:
  DisplayController& _display;
  AudioController&   _audio;
  MotionController&  _motion;
  TouchController&   _touch;
  AlarmController&   _alarm;

  BLEServer*         _pServer;
  BLECharacteristic* _pTxCharacteristic;
  BLECharacteristic* _pRxCharacteristic;

  bool _deviceConnected;
  bool _oldDeviceConnected;
  bool _autoMode;
  unsigned long _lastTelemetryTime;

  void parseCommand(const String& cmdStr);
};

