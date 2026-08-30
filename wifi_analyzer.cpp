#include "wifi_analyzer.h"
#include <WiFi.h>
#include "ui.h"
#include "settings.h"
#include "oui_db.h"

namespace {
WifiSelection selection;
int scanOrder[WIFI_SCAN_MAX];

int signalPercent(int rssi) {
  return constrain(map(rssi, -95, -35, 0, 100), 0, 100);
}

const char* securityName(wifi_auth_mode_t mode) {
  switch (mode) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA1/2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-E";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/3";
    default: return "OTHER";
  }
}

void copySelection(int index) {
  if (index < 0 || index >= WiFi.scanComplete()) return;
  selection.ssid = WiFi.SSID(index);
  selection.bssid = WiFi.BSSIDstr(index);
  selection.channel = static_cast<uint8_t>(WiFi.channel(index));
  selection.rssi = WiFi.RSSI(index);
  selection.security = securityName(WiFi.encryptionType(index));
  selection.manufacturer = ouiVendor(selection.bssid);
}

int makeOrder(int total) {
  const int count = min(total, WIFI_SCAN_MAX);
  for (int i = 0; i < count; ++i) scanOrder[i] = i;

  if (!settings.sortByRssi) return count;
  for (int i = 1; i < count; ++i) {
    const int key = scanOrder[i];
    int j = i - 1;
    while (j >= 0 && WiFi.RSSI(scanOrder[j]) < WiFi.RSSI(key)) {
      scanOrder[j + 1] = scanOrder[j];
      --j;
    }
    scanOrder[j + 1] = key;
  }
  return count;
}

void drawList(int count, int selected, int total) {
  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / NETWORKS", String(total) + " NET");
  const int visible = min(count, 7);
  const int first = min(max(0, selected - 3), max(0, count - visible));

  for (int row = 0; row < visible; ++row) {
    const int pos = first + row;
    const int index = scanOrder[pos];
    const bool active = pos == selected;
    const int y = 39 + row * 24;
    const uint16_t bg = active ? 0x2945 : TFT_BLACK;

    if (active) tft.fillRoundRect(5, y - 3, 310, 21, 3, bg);
    String ssid = WiFi.SSID(index);
    if (ssid.length() == 0) ssid = "<hidden>";
    ssid.replace("\r", " ");
    ssid.replace("\n", " ");
    if (ssid.length() > 18) ssid = ssid.substring(0, 18);

    tft.setTextColor(active ? TFT_WHITE : 0xD6BA, bg);
    tft.drawString(String(pos + 1) + "  " + ssid, 12, y, 1);
    tft.setTextColor(TFT_CYAN, bg);
    tft.drawRightString(String(WiFi.RSSI(index)) + "dBm", 309, y, 1);
    tft.setTextColor(0x7BEF, bg);
    tft.drawString("CH" + String(WiFi.channel(index)) +
                   "  " + String(signalPercent(WiFi.RSSI(index))) + "%  " +
                   securityName(WiFi.encryptionType(index)), 23, y + 11, 1);
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString(String(selected + 1) + "/" + String(count), 8, 195, 1);
  uiFooter();
}

void waitListInput(int count, int& selected) {
  while (true) {
    const ButtonEvent ev = uiReadInput();
    if (ev == BE_PREV) {
      selected = (selected + count - 1) % count;
      drawList(count, selected, count);
    } else if (ev == BE_NEXT) {
      selected = (selected + 1) % count;
      drawList(count, selected, count);
    } else if (ev == BE_SELECT) {
      copySelection(scanOrder[selected]);
      wifiShowSelection();
      drawList(count, selected, count);
    } else if (ev == BE_BACK) {
      return;
    }
    delay(8);
  }
}
}

void wifiScan(bool showHidden) {
  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  WiFi.disconnect(false);

  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / SCAN");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(showHidden ? "Scanning, hidden on" : "Scanning", 8, 52, 1);

  const int total = WiFi.scanNetworks(false, showHidden);
  if (total <= 0) {
    WiFi.scanDelete();
    uiMessage("Nenhuma rede encontrada.", "WIFI / SCAN");
    return;
  }

  const int count = makeOrder(total);
  int selected = 0;
  drawList(count, selected, total);
  waitListInput(count, selected);
  WiFi.scanDelete();
}

void wifiPrintDetails(int index) {
  const int total = WiFi.scanComplete();
  if (total <= 0 || index < 0 || index >= total) {
    uiMessage("O scan atual nao esta disponivel.", "WIFI / DETAIL");
    return;
  }
  copySelection(index);
  wifiShowSelection();
}

void wifiShowSelection() {
  if (!selection.bssid.length()) {
    uiMessage("Nenhuma rede selecionada.", "WIFI / DETAIL");
    return;
  }

  const int frequency = (selection.channel == 14)
                        ? 2484
                        : 2407 + static_cast<int>(selection.channel) * 5;
  String report;
  report.reserve(260);
  report += "SSID: ";
  report += selection.ssid.length() ? selection.ssid : "<hidden>";
  report += "\nBSSID: ";
  report += selection.bssid;
  report += "\nVendor: ";
  report += selection.manufacturer;
  report += "\nChannel: ";
  report += String(selection.channel);
  report += "\nFrequency: ";
  report += String(frequency);
  report += " MHz\nRSSI: ";
  report += String(selection.rssi);
  report += " dBm\nQuality: ";
  report += String(signalPercent(selection.rssi));
  report += "%\nSecurity: ";
  report += selection.security;
  uiMessage(report, "WIFI / DETAIL");
}

WifiSelection wifiLastSelection() { return selection; }
bool wifiHasSelection() { return selection.bssid.length() > 0; }
String wifiLastSelectedSsid() { return selection.ssid; }
String wifiLastSelectedBssid() { return selection.bssid; }
