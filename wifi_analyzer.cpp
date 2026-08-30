#include "wifi_analyzer.h"
#include <WiFi.h>
#include "ui.h"
#include "buttons.h"
#include "touch.h"
#include "settings.h"
#include "oui_db.h"

static WifiSelection selection;

static int signalPercent(int rssi) {
  return constrain(map(rssi, -95, -35, 0, 100), 0, 100);
}

static String securityName(wifi_auth_mode_t mode) {
  switch (mode) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-ENT";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/3";
    default: return "UNKNOWN";
  }
}

static void saveSelection(int index) {
  selection.ssid = WiFi.SSID(index);
  selection.bssid = WiFi.BSSIDstr(index);
  selection.channel = WiFi.channel(index);
  selection.rssi = WiFi.RSSI(index);
  selection.security = securityName(WiFi.encryptionType(index));
  selection.manufacturer = ouiVendor(selection.bssid);
}

static int buildOrder(int total, int* order, int capacity, bool sortByRssi) {
  const int count = min(total, capacity);
  for (int i = 0; i < count; ++i) order[i] = i;
  if (!sortByRssi) return count;
  for (int i = 1; i < count; ++i) {
    const int key = order[i];
    int j = i - 1;
    while (j >= 0 && WiFi.RSSI(order[j]) < WiFi.RSSI(key)) {
      order[j + 1] = order[j];
      --j;
    }
    order[j + 1] = key;
  }
  return count;
}

static void drawScanList(const int* order, int count, int selected, int total) {
  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / NETWORKS");
  const int shown = min(count, 7);
  const int first = min(max(0, selected - 3), max(0, count - shown));

  for (int row = 0; row < shown; ++row) {
    const int position = first + row;
    const int index = order[position];
    const bool active = position == selected;
    String ssid = WiFi.SSID(index);
    if (!ssid.length()) ssid = "<hidden>";
    ssid.replace("\n", " ");
    if (ssid.length() > 18) ssid = ssid.substring(0, 18);
    const int y = 39 + row * 23;
    const uint16_t bg = active ? 0x2945 : TFT_BLACK;
    if (active) tft.fillRoundRect(5, y - 3, 310, 20, 2, bg);
    tft.setTextColor(active ? TFT_WHITE : 0xD6BA, bg);
    tft.drawString(String(position + 1) + "  " + ssid, 12, y);
    tft.setTextColor(TFT_CYAN, bg);
    tft.drawRightString(String(WiFi.RSSI(index)) + "dBm", 310, y, 2);
    tft.setTextColor(0x7BEF, bg);
    tft.drawString("CH " + String(WiFi.channel(index)) + "  " + String(signalPercent(WiFi.RSSI(index))) + "%  " + securityName(WiFi.encryptionType(index)), 22, y + 11);
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString(String(total) + " networks", 8, 196);
  tft.drawRightString(String(selected + 1) + "/" + String(count), 310, 196, 2);
  uiFooter();
}

void wifiScan(bool showHidden) {
  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  WiFi.disconnect(false);
  delay(80);

  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / SCAN");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Scanning 2.4 GHz...", 8, 46);
  tft.drawString(showHidden ? "hidden: on" : "hidden: off", 8, 68);

  const int total = WiFi.scanNetworks(false, showHidden);
  if (total <= 0) {
    WiFi.scanDelete();
    uiMessage("Nenhuma rede encontrada.\n\nVerifique o alcance e tente novamente.", "WIFI / SCAN");
    return;
  }

  static int order[96];
  const int count = buildOrder(total, order, 96, settings.sortByRssi);
  int selected = 0;
  bool done = false;
  drawScanList(order, count, selected, total);

  while (!done) {
    const ButtonEvent be = buttonsRead();
    const TouchEvent te = touchRead();
    if (be == BE_PREV || te == TOUCH_PREV) {
      selected = (selected + count - 1) % count;
      drawScanList(order, count, selected, total);
    } else if (be == BE_NEXT || te == TOUCH_NEXT) {
      selected = (selected + 1) % count;
      drawScanList(order, count, selected, total);
    } else if (be == BE_SELECT || te == TOUCH_SELECT) {
      saveSelection(order[selected]);
      wifiPrintDetails(order[selected]);
      drawScanList(order, count, selected, total);
    } else if (be == BE_BACK || te == TOUCH_BACK) {
      done = true;
    }
    delay(8);
  }

  WiFi.scanDelete();
  uiToast("WiFi scan encerrado");
}

void wifiPrintDetails(int index) {
  const int total = WiFi.scanComplete();
  if (total <= 0 || index < 0 || index >= total) {
    uiMessage("Nenhuma rede disponivel.", "WIFI / DETAIL");
    return;
  }
  saveSelection(index);
  const int frequency = selection.channel == 14 ? 2484 : 2407 + selection.channel * 5;
  uiMessage(
    "SSID: " + (selection.ssid.length() ? selection.ssid : "<hidden>") +
    "\nBSSID: " + selection.bssid +
    "\nFabricante: " + selection.manufacturer +
    "\nCanal: " + String(selection.channel) +
    "\nFrequencia: " + String(frequency) + " MHz" +
    "\nRSSI: " + String(selection.rssi) + " dBm" +
    "\nQualidade: " + String(signalPercent(selection.rssi)) + "%" +
    "\nSeguranca: " + selection.security,
    "WIFI / DETAIL"
  );
}

WifiSelection wifiLastSelection() { return selection; }
bool wifiHasSelection() { return selection.bssid.length() > 0; }
String wifiLastSelectedSsid() { return selection.ssid; }
String wifiLastSelectedBssid() { return selection.bssid; }
