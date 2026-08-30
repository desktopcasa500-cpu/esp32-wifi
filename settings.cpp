#include "settings.h"
#include <Preferences.h>

Preferences prefs;
Settings settings;

void settingsDefaults() {
  settings.wifiScanMs = 5000;
  settings.bleScanMs = 5000;
  settings.diagnosticTimeoutMs = 1500;
  settings.pingsPerDiagnostic = 3;
  settings.brightness = 220;
  settings.defaultChannel = 1;
  settings.theme = 0;
  settings.language = 0;
  settings.hiddenScan = true;
  settings.autoRescan = false;
  settings.activeBleScan = true;
  settings.sortByRssi = true;
  settings.bootAnimation = true;
  settings.debugSerial = true;
  settings.deviceName = "ESP32-WiFi-Toolkit";
  settings.touchMinX = 200;
  settings.touchMaxX = 3900;
  settings.touchMinY = 200;
  settings.touchMaxY = 3900;
}

void settingsLoad() {
  settingsDefaults();
  if (!prefs.begin("wt9", true)) return;

  settings.wifiScanMs = prefs.getUShort("wifi_ms", settings.wifiScanMs);
  settings.bleScanMs = prefs.getUShort("ble_ms", settings.bleScanMs);
  settings.diagnosticTimeoutMs = prefs.getUShort("diag_to", settings.diagnosticTimeoutMs);
  settings.pingsPerDiagnostic = prefs.getUShort("pings", settings.pingsPerDiagnostic);
  settings.brightness = prefs.getUChar("bright", settings.brightness);
  settings.defaultChannel = prefs.getUChar("channel", settings.defaultChannel);
  settings.theme = prefs.getUChar("theme", settings.theme);
  settings.language = prefs.getUChar("lang", settings.language);
  settings.hiddenScan = prefs.getBool("hidden", settings.hiddenScan);
  settings.autoRescan = prefs.getBool("auto", settings.autoRescan);
  settings.activeBleScan = prefs.getBool("ble_act", settings.activeBleScan);
  settings.sortByRssi = prefs.getBool("ble_rssi", settings.sortByRssi);
  settings.bootAnimation = prefs.getBool("boot", settings.bootAnimation);
  settings.debugSerial = prefs.getBool("debug", settings.debugSerial);
  settings.deviceName = prefs.getString("name", settings.deviceName);
  settings.touchMinX = prefs.getShort("tx0", settings.touchMinX);
  settings.touchMaxX = prefs.getShort("tx1", settings.touchMaxX);
  settings.touchMinY = prefs.getShort("ty0", settings.touchMinY);
  settings.touchMaxY = prefs.getShort("ty1", settings.touchMaxY);

  prefs.end();

  settings.wifiScanMs = constrain(settings.wifiScanMs, 1000, 10000);
  settings.bleScanMs = constrain(settings.bleScanMs, 2000, 15000);
  settings.diagnosticTimeoutMs = constrain(settings.diagnosticTimeoutMs, 500, 5000);
  settings.pingsPerDiagnostic = constrain(settings.pingsPerDiagnostic, 1, 10);
  settings.defaultChannel = constrain(settings.defaultChannel, 1, 13);
  settings.brightness = constrain(settings.brightness, 10, 255);
  settings.touchMinX = constrain(settings.touchMinX, 0, 4095);
  settings.touchMaxX = constrain(settings.touchMaxX, 0, 4095);
  settings.touchMinY = constrain(settings.touchMinY, 0, 4095);
  settings.touchMaxY = constrain(settings.touchMaxY, 0, 4095);
}

void settingsSave() {
  if (!prefs.begin("wt9", false)) return;
  prefs.putUShort("wifi_ms", settings.wifiScanMs);
  prefs.putUShort("ble_ms", settings.bleScanMs);
  prefs.putUShort("diag_to", settings.diagnosticTimeoutMs);
  prefs.putUShort("pings", settings.pingsPerDiagnostic);
  prefs.putUChar("bright", settings.brightness);
  prefs.putUChar("channel", settings.defaultChannel);
  prefs.putUChar("theme", settings.theme);
  prefs.putUChar("lang", settings.language);
  prefs.putBool("hidden", settings.hiddenScan);
  prefs.putBool("auto", settings.autoRescan);
  prefs.putBool("ble_act", settings.activeBleScan);
  prefs.putBool("ble_rssi", settings.sortByRssi);
  prefs.putBool("boot", settings.bootAnimation);
  prefs.putBool("debug", settings.debugSerial);
  prefs.putString("name", settings.deviceName);
  prefs.putShort("tx0", settings.touchMinX);
  prefs.putShort("tx1", settings.touchMaxX);
  prefs.putShort("ty0", settings.touchMinY);
  prefs.putShort("ty1", settings.touchMaxY);
  prefs.end();
}

void settingsReset() {
  settingsDefaults();
  settingsSave();
}
