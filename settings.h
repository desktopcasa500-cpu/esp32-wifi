#pragma once
#include <Arduino.h>
struct Settings {
  uint16_t wifiScanMs=5000;
  uint16_t bleScanMs=5000;
  uint8_t brightness=220;
  uint8_t defaultChannel=1;
  bool hiddenScan=true;
  bool autoRescan=false;
  bool debugSerial=true;
  String deviceName="ESP32-WiFi-Toolkit";
};
extern Settings settings;
void settingsLoad();
void settingsSave();
void settingsReset();
