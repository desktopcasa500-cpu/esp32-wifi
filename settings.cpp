#include "settings.h"
#include <Preferences.h>
Preferences prefs;
Settings settings;
void settingsLoad(){
  prefs.begin("wt9", true);
  settings.wifiScanMs=prefs.getUShort("wifi_ms",5000);
  settings.bleScanMs=prefs.getUShort("ble_ms",5000);
  settings.brightness=prefs.getUChar("bright",220);
  settings.defaultChannel=prefs.getUChar("channel",1);
  settings.hiddenScan=prefs.getBool("hidden",true);
  settings.autoRescan=prefs.getBool("auto",false);
  settings.debugSerial=prefs.getBool("debug",true);
  settings.deviceName=prefs.getString("name","ESP32-WiFi-Toolkit");
  prefs.end();
}
void settingsSave(){
  prefs.begin("wt9",false);
  prefs.putUShort("wifi_ms",settings.wifiScanMs); prefs.putUShort("ble_ms",settings.bleScanMs);
  prefs.putUChar("bright",settings.brightness); prefs.putUChar("channel",settings.defaultChannel);
  prefs.putBool("hidden",settings.hiddenScan); prefs.putBool("auto",settings.autoRescan); prefs.putBool("debug",settings.debugSerial);
  prefs.putString("name",settings.deviceName); prefs.end();
}
void settingsReset(){ settings=Settings(); settingsSave(); }
