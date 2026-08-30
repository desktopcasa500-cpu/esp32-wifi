#include "wifi_analyzer.h"
#include <WiFi.h>
#include "ui.h"
#include "buttons.h"
#include "touch.h"

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

static void drawScanList(int selected, int total) {
  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / NETWORKS");

  const int shown = min(total, 7);
  const int first = min(max(0, selected - 3), max(0, total - shown));

  for (int i = 0; i < shown; ++i) {
    const int index = first + i;
    const bool active = index == selected;
    String ssid = WiFi.SSID(index);
    if (!ssid.length()) ssid = "<hidden>";
    ssid.replace("\n", " ");
    if (ssid.length() > 18) ssid = ssid.substring(0, 18);

    const int y = 39 + i * 23;
    if (active) tft.fillRoundRect(5, y - 3, 310, 20, 2, 0x2945);
    tft.setTextColor(active ? TFT_WHITE : 0xD6BA, active ? 0x2945 : TFT_BLACK);
    tft.drawString(String(index + 1) + "  " + ssid, 12, y);
    tft.setTextColor(TFT_CYAN, active ? 0x2945 : TFT_BLACK);
    tft.drawRightString(String(WiFi.RSSI(index)) + "dBm", 310, y);

    tft.setTextColor(0x7BEF, active ? 0x2945 : TFT_BLACK);
    tft.drawString("CH " + String(WiFi.channel(index)) + "  " + String(signalPercent(WiFi.RSSI(index))) + "%  " + securityName(WiFi.encryptionType(index)), 22, y + 11);
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawRightString(String(selected + 1) + "/" + String(total), 310, 196);
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
  tft.drawString(showHidden ? "hidden: on" : "hidden: off", 8, 70);

  const int total = WiFi.scanNetworks(false, showHidden);
  if (total <= 0) {
    uiMessage("Nenhuma rede encontrada.\n\nVerifique o alcance e tente novamente.");
    WiFi.scanDelete();
    return;
  }

  int selected = 0;
  bool done = false;
  drawScanList(selected, total);

  while (!done) {
    ButtonEvent be = buttonsRead();
    TouchEvent te = touchRead();

    if (be == BE_PREV || te == TOUCH_PREV) {
      selected = (selected + total - 1) % total;
      drawScanList(selected, total);
    } else if (be == BE_NEXT || te == TOUCH_NEXT) {
      selected = (selected + 1) % total;
      drawScanList(selected, total);
    } else if (be == BE_SELECT || te == TOUCH_SELECT) {
      wifiPrintDetails(selected);
      delay(120);
      drawScanList(selected, total);
    } else if (be == BE_BACK || te == TOUCH_BACK) {
      done = true;
    }
    delay(8);
  }

  WiFi.scanDelete();
  uiToast("Scan encerrado");
}

void wifiPrintDetails(int index) {
  const int total = WiFi.scanComplete();
  if (total <= 0 || index < 0 || index >= total) {
    uiMessage("Nenhuma rede disponivel.");
    return;
  }

  String ssid = WiFi.SSID(index);
  if (!ssid.length()) ssid = "<hidden>";
  const int channel = WiFi.channel(index);
  const int frequency = channel == 14 ? 2484 : 2407 + channel * 5;

  uiMessage(
    "SSID: " + ssid +
    "\nBSSID: " + WiFi.BSSIDstr(index) +
    "\nCanal: " + String(channel) +
    "\nFrequencia: " + String(frequency) + " MHz" +
    "\nRSSI: " + String(WiFi.RSSI(index)) + " dBm" +
    "\nQualidade: " + String(signalPercent(WiFi.RSSI(index))) + "%" +
    "\nSeguranca: " + securityName(WiFi.encryptionType(index)),
    "WIFI / DETAIL"
  );
}
