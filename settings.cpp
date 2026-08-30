#include "settings.h"
#include "config.h"
#include <Preferences.h>

Preferences prefs;
Settings settings;

void settingsDefaults() {
  settings.wifiScanMs = DEFAULT_WIFI_SCAN_MS;
  settings.bleScanMs = DEFAULT_BLE_SCAN_MS;
  settings.diagnosticTimeoutMs = DEFAULT_DIAG_TIMEOUT_MS;
  settings.pingsPerDiagnostic = DEFAULT_DIAG_PINGS;
  settings.brightness = DEFAULT_BRIGHTNESS;
  settings.defaultChannel = DEFAULT_WIFI_CHANNEL;
  settings.theme = 0;
  settings.language = 0;
  settings.hiddenScan = true;
  settings.autoRescan = false;
  settings.activeBleScan = true;
  settings.sortByRssi = true;
  settings.bootAnimation = true;
  settings.debugSerial = true;
  settings.deviceName = "ESP32-WiFi-Toolkit";
  settings.touchMinX = DEFAULT_TOUCH_MIN_X;
  settings.touchMaxX = DEFAULT_TOUCH_MAX_X;
  settings.touchMinY = DEFAULT_TOUCH_MIN_Y;
  settings.touchMaxY = DEFAULT_TOUCH_MAX_Y;
}

static void clampSettings() {
  settings.wifiScanMs = constrain(settings.wifiScanMs, 1000, 10000);
  settings.bleScanMs = constrain(settings.bleScanMs, 2000, 15000);
  settings.diagnosticTimeoutMs = constrain(settings.diagnosticTimeoutMs, 500, 5000);
  settings.pingsPerDiagnostic = constrain(settings.pingsPerDiagnostic, 1, 10);
  settings.brightness = constrain(settings.brightness, 10, 255);
  settings.defaultChannel = constrain(settings.defaultChannel, 1, 13);
  settings.theme = constrain(settings.theme, 0, 3);
  settings.language = constrain(settings.language, 0, 1);
  settings.deviceName.trim();
  if (settings.deviceName.length() == 0) settings.deviceName = "ESP32-WiFi-Toolkit";
  if (settings.deviceName.length() > 20) settings.deviceName = settings.deviceName.substring(0, 20);

  if (settings.touchMinX < 0 || settings.touchMaxX > 4095 ||
      settings.touchMinX + 200 >= settings.touchMaxX) {
    settings.touchMinX = DEFAULT_TOUCH_MIN_X;
    settings.touchMaxX = DEFAULT_TOUCH_MAX_X;
  }
  if (settings.touchMinY < 0 || settings.touchMaxY > 4095 ||
      settings.touchMinY + 200 >= settings.touchMaxY) {
    settings.touchMinY = DEFAULT_TOUCH_MIN_Y;
    settings.touchMaxY = DEFAULT_TOUCH_MAX_Y;
  }
}

void settingsLoad() {
  settingsDefaults();
  if (!prefs.begin(SETTINGS_NAMESPACE, true)) return;

  const uint8_t version = prefs.getUChar("version", 0);
  if (version == SETTINGS_VERSION) {
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
  }
  prefs.end();
  clampSettings();
}

void settingsSave() {
  if (!prefs.begin(SETTINGS_NAMESPACE, false)) return;
  clampSettings();
  prefs.putUChar("version", SETTINGS_VERSION);
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
