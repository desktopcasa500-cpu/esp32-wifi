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
  "WiFi Analyzer", "Signal Meter", "Channel Scanner", "Spectrum 2.4G",
  "Hidden Networks", "Network Detail", "Channel Optimizer", "WiFi Diagnostic",
  "Captive Portal", "Packet Monitor", "Deauth Detector", "BLE Scanner",
  "BLE Beacons", "BLE GATT", "BLE Proximity", "BLE Advertiser",
  "Password Strength", "System Info", "Settings", "About"
};
static const size_t menuCount = sizeof(menu) / sizeof(menu[0]);

static ButtonEvent readButton() {
  const ButtonEvent b = buttonsRead();
  if (b != BE_NONE) return b;
  switch (touchRead()) {
    case TOUCH_PREV: return BE_PREV;
    case TOUCH_SELECT: return BE_SELECT;
    case TOUCH_NEXT: return BE_NEXT;
    case TOUCH_BACK: return BE_BACK;
    default: return BE_NONE;
  }
}

static String yesNo(bool value) { return value ? "ON" : "OFF"; }

static void showSystem() {
  uiMessage(
    "ESP32 WiFi Toolkit v9.0\n\n"
    "CPU: " + String(getCpuFrequencyMhz()) + " MHz\n" +
    "Heap: " + String(ESP.getFreeHeap()) + " B\n" +
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
  const int first = min(max(0, selected - 3), max(0, count - 7));

  tft.fillScreen(TFT_BLACK);
  uiHeader("SETTINGS");
  for (int i = first; i < min(count, first + 7); ++i) {
    const bool active = i == selected;
    const int y = 40 + (i - first) * 23;
    const uint16_t bg = active ? 0x2945 : TFT_BLACK;
    if (active) tft.fillRoundRect(5, y - 3, 310, 20, 2, bg);
    tft.setTextColor(active ? TFT_WHITE : 0xD6BA, bg);
    tft.drawString(names[i], 12, y);

    String value;
    switch (i) {
      case 0: value = String(settings.wifiScanMs / 1000) + "s"; break;
      case 1: value = String(settings.bleScanMs / 1000) + "s"; break;
      case 2: value = String(settings.brightness); break;
      case 3: value = "CH " + String(settings.defaultChannel); break;
      case 4: value = yesNo(settings.hiddenScan); break;
      case 5: value = yesNo(settings.autoRescan); break;
      case 6: value = yesNo(settings.activeBleScan); break;
      case 7: value = yesNo(settings.sortByRssi); break;
      case 8: value = yesNo(settings.bootAnimation); break;
      case 9: value = yesNo(settings.debugSerial); break;
      case 10: value = "DEFAULTS"; break;
    }
    tft.setTextColor(active ? TFT_CYAN : 0x7BEF, bg);
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
    const PacketStats s = packetMonitorStats();
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
    tft.drawString("BACK encerra", 10, 188);
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
    tft.drawString("Monitoramento passivo", 10, 165);
    tft.drawString("BACK encerra", 10, 185);
    uiFooter();
    if (readButton() == BE_BACK) break;
    delay(80);
  }
  deauthDetectorStop();
}

static void runCaptivePortal() {
  if (!wifiHasSelection()) {
    uiMessage("Selecione uma rede no WiFi Analyzer primeiro.", "WIFI / CAPTIVE");
    return;
  }
  uiMessage("Rede: " + wifiLastSelectedSsid() + "\n\nA senha sera lida pela Serial e nao sera salva.\nDigite a senha e pressione ENTER.", "WIFI / CAPTIVE");
  while (!Serial.available()) {
    if (readButton() == BE_BACK) return;
    delay(20);
  }
  const String password = Serial.readStringUntil('\n');
  String report;
  detectCaptivePortal(wifiLastSelectedSsid(), password, report);
  uiMessage(report, "WIFI / CAPTIVE RESULT");
}

static void runDiagnostic() {
  if (!wifiHasSelection()) {
    uiMessage("Selecione uma rede no WiFi Analyzer primeiro.", "WIFI / DIAGNOSTIC");
    return;
  }
  uiMessage("Rede: " + wifiLastSelectedSsid() + "\n\nA senha sera lida pela Serial e nao sera salva.\nDigite a senha e pressione ENTER.", "WIFI / DIAGNOSTIC");
  while (!Serial.available()) {
    if (readButton() == BE_BACK) return;
    delay(20);
  }
  const String password = Serial.readStringUntil('\n');
  runWifiDiagnostic(wifiLastSelectedSsid(), password);
}

static void selectMenu() {
  switch (menuIndex) {
    case 0: wifiScan(settings.hiddenScan); break;
    case 1:
      if (wifiHasSelection()) showSignalMeter(wifiLastSelectedSsid(), wifiLastSelectedBssid(), 30000);
      else uiMessage("Selecione uma rede no WiFi Analyzer primeiro.", "WIFI / SIGNAL");
      break;
    case 2: showChannelScan(); break;
    case 3: showWifiSpectrum(); break;
    case 4: showHiddenNetworks(); break;
    case 5: if (wifiHasSelection()) showNetworkDetail(0); else uiMessage("Selecione uma rede no WiFi Analyzer primeiro.", "WIFI / DETAIL"); break;
    case 6: showChannelOptimizer(); break;
    case 7: runDiagnostic(); break;
    case 8: runCaptivePortal(); break;
    case 9: runPacketMonitor(); break;
    case 10: runDeauthDetector(); break;
    case 11: bleScan(settings.bleScanMs / 1000); break;
    case 12: beaconDetect(settings.bleScanMs / 1000); break;
    case 13:
      if (bleHasSelection()) bleInspectDevice(bleLastSelectedAddress());
      else uiMessage("Nenhum alvo BLE selecionado.\n\nAbra BLE Scanner e selecione um dispositivo primeiro.", "BLE / GATT");
      break;
    case 14:
      if (bleHasSelection()) bleProximityMonitor(bleLastSelectedAddress(), 15000);
      else uiMessage("Nenhum alvo BLE selecionado.\n\nAbra BLE Scanner e selecione um dispositivo primeiro.", "BLE / PROXIMITY");
      break;
    case 15:
      bleAdvertiserStart(0, 500);
      uiMessage("Advertiser ativo\n\nPerfil: generico\nIntervalo: 500 ms\nNome: " + settings.deviceName + "\n\nBACK retorna ao menu.", "BLE / ADVERTISER");
      bleAdvertiserStop();
      break;
    case 16: {
      PasswordReport r = analyzePassword("exemplo-senha-123");
      uiMessage("Analise offline\nScore: " + String(r.score) + "/100\nEntropia: " + String(r.entropy, 1) + " bits\nComum: " + String(r.common ? "sim" : "nao") + "\nSequencia: " + String(r.hasSequence ? "sim" : "nao") + "\n\n" + passwordAdvice(r), "SECURITY / PASSWORD");
      break;
    }
    case 17: showSystem(); break;
    case 18: runSettings(); break;
    case 19: uiMessage("ESP32 WiFi Toolkit v9.0\n\nDiagnostico WiFi/BLE, observacao passiva e utilitarios locais.\n\nSem ataques, captura de credenciais ou interferencia intencional.", "ABOUT"); break;
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
