#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "settings.h"
#include "ui.h"
#include "buttons.h"
#include "touch.h"
#include "wifi_analyzer.h"
#include "wifi_channel_scan.h"
#include "wifi_signal_meter.h"
#include "wifi_spectrum.h"
#include "wifi_hidden_detect.h"
#include "channel_optimizer.h"
#include "wifi_diagnostic.h"
#include "packet_monitor.h"
#include "deauth_detect.h"
#include "bt_scanner.h"
#include "ble_inspector.h"
#include "ble_proximity.h"
#include "ble_beacon.h"
#include "ble_advertiser.h"
#include "captive_detect.h"
#include "password_strength.h"

namespace {
int menuIndex = 0;

const char* const MENU[] = {
  "WiFi Analyzer", "Signal Meter", "Channel Scanner", "Spectrum 2.4G",
  "Hidden Networks", "Network Detail", "Channel Optimizer", "WiFi Diagnostic",
  "Captive Portal", "Packet Monitor", "Deauth Detector", "BLE Scanner",
  "BLE Beacons", "BLE GATT", "BLE Proximity", "BLE Advertiser",
  "Password Strength", "System Info", "Settings", "About"
};
const size_t MENU_COUNT = sizeof(MENU) / sizeof(MENU[0]);

void renderMenu() {
  uiMenu(MENU, MENU_COUNT, menuIndex);
}

bool readLineFromSerial(String& out, uint32_t timeoutMs) {
  out = "";
  Serial.setTimeout(50);
  const uint32_t start = millis();
  while ((uint32_t)(millis() - start) < timeoutMs) {
    while (Serial.available()) {
      const char c = static_cast<char>(Serial.read());
      if (c == '\n' || c == '\r') {
        if (out.length()) return true;
      } else if (out.length() < 63) {
        out += c;
      }
    }
    if (uiReadInput() == BE_BACK) return false;
    delay(10);
  }
  return out.length() > 0;
}

bool askPassword(const String& ssid, String& password, const String& title) {
  while (Serial.available()) Serial.read();
  uiMessage("Rede: " + ssid + "\n\nDigite a senha no monitor serial e pressione ENTER.\nA senha nao sera armazenada.", title);
  return readLineFromSerial(password, 60000);
}

void showSystem() {
  const uint32_t flash = ESP.getFlashChipSize();
  String text;
  text.reserve(220);
  text += "Toolkit v9.0\n\n";
  text += "CPU: " + String(getCpuFrequencyMhz()) + " MHz\n";
  text += "Heap: " + String(ESP.getFreeHeap()) + " B\n";
  text += "Heap minimo: " + String(ESP.getMinFreeHeap()) + " B\n";
  text += "Flash: " + String(flash / 1024 / 1024) + " MB\n";
  text += "MAC: " + WiFi.macAddress() + "\n";
  text += "Uptime: " + String(millis() / 1000) + " s";
  uiMessage(text, "SYSTEM / STATUS");
}

void drawSettings(int selected) {
  const char* const names[] = {
    "WiFi scan", "BLE scan", "Brightness", "Default channel", "Hidden networks",
    "Auto rescan", "BLE active scan", "Sort by RSSI", "Boot animation", "Serial debug",
    "Touch calibration", "Factory reset"
  };
  const int count = sizeof(names) / sizeof(names[0]);
  const int first = min(max(0, selected - 3), max(0, count - 7));

  tft.fillScreen(TFT_BLACK);
  uiHeader("SETTINGS");
  for (int i = first; i < min(count, first + 7); ++i) {
    const bool active = i == selected;
    const int y = 39 + (i - first) * 24;
    const uint16_t bg = active ? 0x2945 : TFT_BLACK;
    if (active) tft.fillRoundRect(5, y - 3, 310, 21, 3, bg);
    tft.setTextColor(active ? TFT_WHITE : 0xD6BA, bg);
    tft.drawString(names[i], 12, y, 1);

    String value;
    switch (i) {
      case 0: value = String(settings.wifiScanMs / 1000) + "s"; break;
      case 1: value = String(settings.bleScanMs / 1000) + "s"; break;
      case 2: value = String(settings.brightness); break;
      case 3: value = "CH " + String(settings.defaultChannel); break;
      case 4: value = settings.hiddenScan ? "ON" : "OFF"; break;
      case 5: value = settings.autoRescan ? "ON" : "OFF"; break;
      case 6: value = settings.activeBleScan ? "ON" : "OFF"; break;
      case 7: value = settings.sortByRssi ? "ON" : "OFF"; break;
      case 8: value = settings.bootAnimation ? "ON" : "OFF"; break;
      case 9: value = settings.debugSerial ? "ON" : "OFF"; break;
      case 10: value = "RUN"; break;
      case 11: value = "RESET"; break;
    }
    tft.setTextColor(active ? TFT_CYAN : 0x7BEF, bg);
    tft.drawRightString(value, 308, y, 1);
  }
  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString(String(selected + 1) + "/" + String(count), 8, 194, 1);
  uiFooter();
}

void runSettings() {
  int selected = 0;
  drawSettings(selected);
  while (true) {
    const ButtonEvent ev = uiReadInput();
    if (ev == BE_PREV) {
      selected = (selected + 11) % 12;
      drawSettings(selected);
    } else if (ev == BE_NEXT) {
      selected = (selected + 1) % 12;
      drawSettings(selected);
    } else if (ev == BE_SELECT) {
      switch (selected) {
        case 0: settings.wifiScanMs = settings.wifiScanMs >= 10000 ? 1000 : settings.wifiScanMs + 1000; break;
        case 1: settings.bleScanMs = settings.bleScanMs >= 15000 ? 2000 : settings.bleScanMs + 1000; break;
        case 2: settings.brightness = settings.brightness >= 240 ? 40 : settings.brightness + 20; uiSetBrightness(settings.brightness); break;
        case 3: settings.defaultChannel = settings.defaultChannel >= 13 ? 1 : settings.defaultChannel + 1; break;
        case 4: settings.hiddenScan = !settings.hiddenScan; break;
        case 5: settings.autoRescan = !settings.autoRescan; break;
        case 6: settings.activeBleScan = !settings.activeBleScan; break;
        case 7: settings.sortByRssi = !settings.sortByRssi; break;
        case 8: settings.bootAnimation = !settings.bootAnimation; break;
        case 9: settings.debugSerial = !settings.debugSerial; break;
        case 10: {
          TouchCalibration c = {settings.touchMinX, settings.touchMaxX, settings.touchMinY, settings.touchMaxY};
          if (touchCalibrate(c)) {
            settings.touchMinX = c.minX; settings.touchMaxX = c.maxX;
            settings.touchMinY = c.minY; settings.touchMaxY = c.maxY;
          }
          break;
        }
        case 11:
          settingsReset();
          touchSetCalibration({settings.touchMinX, settings.touchMaxX, settings.touchMinY, settings.touchMaxY});
          uiSetBrightness(settings.brightness);
          break;
      }
      settingsSave();
      drawSettings(selected);
    } else if (ev == BE_BACK) {
      return;
    }
    delay(8);
  }
}

void runPacketMonitor() {
  packetMonitorStart(settings.defaultChannel);
  const uint32_t start = millis();
  while ((uint32_t)(millis() - start) < 30000U) {
    const PacketStats s = packetMonitorStats();
    tft.fillScreen(TFT_BLACK);
    uiHeader("WIFI / PACKETS", "PASSIVE");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Total  " + String(s.total), 10, 47, 2);
    tft.drawString("Beacon " + String(s.beacon), 10, 72, 1);
    tft.drawString("Probe  " + String(s.probe), 10, 91, 1);
    tft.drawString("Mgmt   " + String(s.mgmt), 10, 110, 1);
    tft.drawString("Data   " + String(s.data), 160, 72, 1);
    tft.drawString("Ctrl   " + String(s.control), 160, 91, 1);
    tft.drawString("PPS    " + String(s.pps), 160, 110, 1);
    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.drawString("Channel " + String(settings.defaultChannel), 10, 146, 1);
    tft.drawString("BACK to exit", 10, 188, 1);
    uiFooter();
    if (uiReadInput() == BE_BACK) break;
    delay(80);
  }
  packetMonitorStop();
}

void runDeauthDetector() {
  deauthDetectorStart(settings.defaultChannel);
  const uint32_t start = millis();
  while ((uint32_t)(millis() - start) < 30000U) {
    tft.fillScreen(TFT_BLACK);
    uiHeader("WIFI / MGMT EVENTS", "PASSIVE");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Deauth    " + String(deauthEvents()), 10, 55, 1);
    tft.drawString("Disassoc  " + String(disassocEvents()), 10, 78, 1);
    tft.drawString("Last RSSI " + String(deauthLastRssi()) + " dBm", 10, 101, 1);
    tft.drawString("Channel   " + String(deauthChannel()), 10, 124, 1);
    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.drawString("Observation only", 10, 158, 1);
    uiFooter();
    if (uiReadInput() == BE_BACK) break;
    delay(80);
  }
  deauthDetectorStop();
}

void selectMenu() {
  switch (menuIndex) {
    case 0: wifiScan(settings.hiddenScan); break;
    case 1: if (wifiHasSelection()) showSignalMeter(wifiLastSelectedSsid(), wifiLastSelectedBssid(), 30000); else uiMessage("Select a network first.", "WIFI / SIGNAL"); break;
    case 2: showChannelScan(); break;
    case 3: showWifiSpectrum(); break;
    case 4: showHiddenNetworks(); break;
    case 5: if (wifiHasSelection()) wifiPrintDetails(0); else uiMessage("Select a network first.", "WIFI / DETAIL"); break;
    case 6: showChannelOptimizer(); break;
    case 7: {
      if (!wifiHasSelection()) { uiMessage("Select a network first.", "WIFI / DIAGNOSTIC"); break; }
      String pass;
      if (askPassword(wifiLastSelectedSsid(), pass, "WIFI / DIAGNOSTIC")) runWifiDiagnostic(wifiLastSelectedSsid(), pass);
      break;
    }
    case 8: {
      if (!wifiHasSelection()) { uiMessage("Select a network first.", "WIFI / CAPTIVE"); break; }
      String pass;
      if (askPassword(wifiLastSelectedSsid(), pass, "WIFI / CAPTIVE")) {
        String report;
        detectCaptivePortal(wifiLastSelectedSsid(), pass, report);
        uiMessage(report, "WIFI / CAPTIVE RESULT");
      }
      break;
    }
    case 9: runPacketMonitor(); break;
    case 10: runDeauthDetector(); break;
    case 11: bleScan(settings.bleScanMs / 1000); break;
    case 12: beaconDetect(settings.bleScanMs / 1000); break;
    case 13: if (bleHasSelection()) bleInspectDevice(bleLastSelectedAddress()); else uiMessage("Select a BLE device first.", "BLE / GATT"); break;
    case 14: if (bleHasSelection()) bleProximityMonitor(bleLastSelectedAddress(), 30000); else uiMessage("Select a BLE device first.", "BLE / PROXIMITY"); break;
    case 15:
      bleAdvertiserStart(0, 500);
      uiMessage("BLE advertiser active.\n\nName: " + settings.deviceName + "\nInterval: 500 ms\n\nBACK to stop.", "BLE / ADVERTISER");
      bleAdvertiserStop();
      break;
    case 16: {
      String password;
      while (Serial.available()) Serial.read();
      uiMessage("Digite uma senha de teste no Serial.\nEla sera analisada apenas localmente.", "SECURITY / PASSWORD");
      if (readLineFromSerial(password, 60000)) {
        const PasswordReport r = analyzePassword(password);
        String report = "Score: " + String(r.score) + "/100\n";
        report += "Entropy: " + String(r.entropy, 1) + " bits\n";
        report += "Common: " + String(r.common ? "yes" : "no") + "\n";
        report += "Sequence: " + String(r.hasSequence ? "yes" : "no") + "\n\n";
        report += passwordAdvice(r);
        uiMessage(report, "SECURITY / PASSWORD");
      }
      break;
    }
    case 17: showSystem(); break;
    case 18: runSettings(); break;
    case 19: uiMessage("ESP32 WiFi Toolkit v9.0\n\nWiFi/BLE diagnostics and passive observation.\n\nNo attacks, credential capture or intentional interference.", "ABOUT"); break;
  }
  delay(100);
  renderMenu();
}
}

void setup() {
  Serial.begin(115200);
  settingsLoad();
  buttonsBegin();
  uiBegin();
  touchBegin();
  touchSetCalibration({settings.touchMinX, settings.touchMaxX, settings.touchMinY, settings.touchMaxY});
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false);
  delay(100);
  renderMenu();
}

void loop() {
  switch (uiReadInput()) {
    case BE_PREV:
      menuIndex = (menuIndex + MENU_COUNT - 1) % MENU_COUNT;
      renderMenu();
      break;
    case BE_NEXT:
      menuIndex = (menuIndex + 1) % MENU_COUNT;
      renderMenu();
      break;
    case BE_SELECT:
      selectMenu();
      break;
    default:
      break;
  }
  delay(10);
}
