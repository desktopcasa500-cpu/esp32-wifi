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
#include "network_detail.h"
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

static int menuIndex = 0;
static const String menu[] = {
  "WiFi Analyzer", "Channel Scanner", "Spectrum 2.4G", "Hidden Networks",
  "Channel Optimizer", "Packet Monitor", "Deauth Detector", "BLE Scanner",
  "BLE Beacons", "BLE GATT", "BLE Proximity", "BLE Advertiser",
  "Password Strength", "System Info", "Settings", "About"
};
static const size_t menuCount = sizeof(menu) / sizeof(menu[0]);

static ButtonEvent readButton() {
  ButtonEvent b = buttonsRead();
  if (b != BE_NONE) return b;
  switch (touchRead()) {
    case TOUCH_PREV: return BE_PREV;
    case TOUCH_SELECT: return BE_SELECT;
    case TOUCH_NEXT: return BE_NEXT;
    case TOUCH_BACK: return BE_BACK;
    default: return BE_NONE;
  }
}

static String boolText(bool value) { return value ? "ON" : "OFF"; }

static void showSystem() {
  uiMessage(
    "ESP32 WiFi Toolkit v9.0\n\n"
    "CPU: " + String(getCpuFrequencyMhz()) + " MHz\n" +
    "Heap livre: " + String(ESP.getFreeHeap()) + " B\n" +
    "Heap minimo: " + String(ESP.getMinFreeHeap()) + " B\n" +
    "Flash: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB\n" +
    "MAC: " + WiFi.macAddress() + "\n" +
    "Uptime: " + String(millis() / 1000) + " s",
    "SYSTEM / STATUS"
  );
}

static void drawSettings(int selected) {
  const String names[] = {
    "WiFi scan", "BLE scan", "Brilho", "Canal padrao", "Redes ocultas",
    "Auto-rescan", "BLE ativo", "Ordenar por RSSI", "Boot", "Debug", "Reset"
  };
  const int count = sizeof(names) / sizeof(names[0]);

  tft.fillScreen(TFT_BLACK);
  uiHeader("SETTINGS");
  const int first = min(max(0, selected - 3), max(0, count - 7));
  const int last = min(count, first + 7);

  for (int i = first; i < last; ++i) {
    const bool active = i == selected;
    const int y = 40 + (i - first) * 23;
    if (active) tft.fillRoundRect(5, y - 3, 310, 20, 2, 0x2945);
    tft.setTextColor(active ? TFT_WHITE : 0xD6BA, active ? 0x2945 : TFT_BLACK);
    tft.drawString(names[i], 12, y);

    String value;
    switch (i) {
      case 0: value = String(settings.wifiScanMs / 1000) + "s"; break;
      case 1: value = String(settings.bleScanMs / 1000) + "s"; break;
      case 2: value = String(settings.brightness); break;
      case 3: value = "CH " + String(settings.defaultChannel); break;
      case 4: value = boolText(settings.hiddenScan); break;
      case 5: value = boolText(settings.autoRescan); break;
      case 6: value = boolText(settings.activeBleScan); break;
      case 7: value = boolText(settings.sortByRssi); break;
      case 8: value = boolText(settings.bootAnimation); break;
      case 9: value = boolText(settings.debugSerial); break;
      case 10: value = "DEFAULTS"; break;
    }
    tft.setTextColor(active ? TFT_CYAN : 0x7BEF, active ? 0x2945 : TFT_BLACK);
    tft.drawRightString(value, 307, y);
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawRightString(String(selected + 1) + "/" + String(count), 310, 196);
  uiFooter();
}

static void runSettings() {
  int selected = 0;
  bool done = false;
  drawSettings(selected);

  while (!done) {
    const ButtonEvent ev = readButton();
    if (ev == BE_PREV) {
      selected = (selected + 10) % 11;
      drawSettings(selected);
    } else if (ev == BE_NEXT) {
      selected = (selected + 1) % 11;
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
        case 10: settingsReset(); uiToast("Defaults restored"); break;
      }
      settingsSave();
      drawSettings(selected);
    } else if (ev == BE_BACK) {
      done = true;
    }
    delay(8);
  }
}

static void runPacketMonitor() {
  packetMonitorStart(settings.defaultChannel);
  const uint32_t until = millis() + 5000;
  while (millis() < until) {
    PacketStats s = packetMonitorStats();
    tft.fillScreen(TFT_BLACK);
    uiHeader("WIFI / PACKETS", "LIVE");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Total     " + String(s.total), 10, 45);
    tft.drawString("Beacon    " + String(s.beacon), 10, 67);
    tft.drawString("Probe     " + String(s.probe), 10, 89);
    tft.drawString("Mgmt      " + String(s.mgmt), 10, 111);
    tft.drawString("Data      " + String(s.data), 160, 45);
    tft.drawString("Control   " + String(s.control), 160, 67);
    tft.drawString("PPS       " + String(s.pps), 160, 89);
    tft.drawString("CH        " + String(settings.defaultChannel), 160, 111);
    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.drawString("BACK para sair", 10, 190);
    uiFooter();
    if (readButton() == BE_BACK) break;
    delay(80);
  }
  packetMonitorStop();
}

static void runDeauthDetector() {
  deauthDetectorStart(settings.defaultChannel);
  const uint32_t until = millis() + 5000;
  while (millis() < until) {
    tft.fillScreen(TFT_BLACK);
    uiHeader("WIFI / DEAUTH", "PASSIVE");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Deauth       " + String(deauthEvents()), 10, 55);
    tft.drawString("Disassoc     " + String(disassocEvents()), 10, 80);
    tft.drawString("Last RSSI    " + String(deauthLastRssi()) + " dBm", 10, 105);
    tft.drawString("Channel      " + String(deauthChannel()), 10, 130);
    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.drawString("Somente monitoramento passivo", 10, 165);
    tft.drawString("BACK encerra", 10, 185);
    uiFooter();
    if (readButton() == BE_BACK) break;
    delay(80);
  }
  deauthDetectorStop();
}

static void selectMenu() {
  switch (menuIndex) {
    case 0: wifiScan(settings.hiddenScan); break;
    case 1: showChannelScan(); break;
    case 2: showWifiSpectrum(); break;
    case 3: showHiddenNetworks(); break;
    case 4: showChannelOptimizer(); break;
    case 5: runPacketMonitor(); break;
    case 6: runDeauthDetector(); break;
    case 7: bleScan(settings.bleScanMs / 1000); break;
    case 8: beaconDetect(settings.bleScanMs / 1000); break;
    case 9: bleInspectDevice("00:00:00:00:00:00"); break;
    case 10: bleProximityMonitor("00:00:00:00:00:00", 5000); break;
    case 11: bleAdvertiserStart(0, 500); uiMessage("BLE advertiser ativo.\nPerfil: Generico\nIntervalo: 500 ms\n\nBACK retorna ao menu.", "BLE / ADVERTISER"); bleAdvertiserStop(); break;
    case 12: { PasswordReport r = analyzePassword("exemplo-senha-123"); uiMessage("Analise offline\nScore: " + String(r.score) + "/100\nEntropia: " + String(r.entropy, 1) + " bits\nComum: " + String(r.common ? "sim" : "nao") + "\nSequencia: " + String(r.hasSequence ? "sim" : "nao") + "\n\n" + passwordAdvice(r), "SECURITY / PASSWORD"); } break;
    case 13: showSystem(); break;
    case 14: runSettings(); break;
    case 15: uiMessage("ESP32 WiFi Toolkit v9.0\n\nDiagnostico WiFi/BLE, observacao passiva e utilitarios locais.\n\nNao realiza ataques ou captura de credenciais.", "ABOUT"); break;
  }
  delay(150);
  render();
}

void render() { uiMenu(menu, menuCount, menuIndex); }

void setup() {
  Serial.begin(115200);
  settingsLoad();
  uiBegin();
  buttonsBegin();
  touchBegin();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);
  render();
}

void loop() {
  switch (readButton()) {
    case BE_PREV: menuIndex = (menuIndex + menuCount - 1) % menuCount; render(); break;
    case BE_NEXT: menuIndex = (menuIndex + 1) % menuCount; render(); break;
    case BE_SELECT: selectMenu(); break;
    case BE_BACK: render(); break;
    default: break;
  }
  delay(10);
}
