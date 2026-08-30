#pragma once
#include <Arduino.h>

void wifiScan(bool showHidden = true);
void wifiPrintDetails(int index);
String wifiLastSelectedSsid();
String wifiLastSelectedBssid();
bool wifiHasSelection();
