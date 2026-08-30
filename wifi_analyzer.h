#pragma once
#include <Arduino.h>

struct WifiSelection {
  String ssid;
  String bssid;
  uint8_t channel;
  int rssi;
  String security;
};

void wifiScan(bool showHidden = true);
void wifiPrintDetails(int index);
WifiSelection wifiLastSelection();
bool wifiHasSelection();
String wifiLastSelectedSsid();
String wifiLastSelectedBssid();
