#pragma once
#include <Arduino.h>

struct Settings {
  uint16_t wifiScanMs;
  uint16_t bleScanMs;
  uint16_t diagnosticTimeoutMs;
  uint16_t pingsPerDiagnostic;
  uint8_t brightness;
  uint8_t defaultChannel;
  uint8_t theme;
  uint8_t language;
  bool hiddenScan;
  bool autoRescan;
  bool activeBleScan;
  bool sortByRssi;
  bool bootAnimation;
  bool debugSerial;
  String deviceName;
};

extern Settings settings;
void settingsDefaults();
void settingsLoad();
void settingsSave();
void settingsReset();
