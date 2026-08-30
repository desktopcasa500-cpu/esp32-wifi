#include "wifi_analyzer.h"
#include <WiFi.h>
#include "ui.h"

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

static int signalPercent(int rssi) {
  return constrain(map(rssi, -95, -35, 0, 100), 0, 100);
}

void wifiScan(bool showHidden) {
  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  WiFi.disconnect(false);
  delay(80);

  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / SCAN");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Scanning 2.4 GHz...", 8, 42);

  const int n = WiFi.scanNetworks(false, showHidden);
  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / NETWORKS");

  if (n <= 0) {
    uiMessage("Nenhuma rede encontrada.\n\nTente novamente ou aproxime-se do ponto de acesso.");
    return;
  }

  const int shown = min(n, 7);
  for (int i = 0; i < shown; ++i) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) ssid = "<hidden>";
    ssid.replace("\n", " ");
    if (ssid.length() > 19) ssid = ssid.substring(0, 19);

    const int y = 38 + i * 24;
    const int rssi = WiFi.RSSI(i);
    const int quality = signalPercent(rssi);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(i + 1) + " " + ssid, 5, y);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawRightString(String(rssi) + "dBm", 316, y);
    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.drawString("CH " + String(WiFi.channel(i)) + "  " + String(quality) + "%  " + securityName(WiFi.encryptionType(i)), 14, y + 12);
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString(String(n) + " networks", 8, 198);
  uiFooter();
}

void wifiPrintDetails(int index) {
  const int total = WiFi.scanComplete();
  if (total <= 0 || index < 0 || index >= total) {
    uiMessage("Nenhuma rede disponível.");
    return;
  }

  String ssid = WiFi.SSID(index);
  if (ssid.length() == 0) ssid = "<hidden>";

  const int channel = WiFi.channel(index);
  const int frequency = (channel == 14) ? 2484 : 2407 + channel * 5;

  uiMessage(
    "SSID: " + ssid +
    "\nBSSID: " + WiFi.BSSIDstr(index) +
    "\nCanal: " + String(channel) +
    "\nFrequencia: " + String(frequency) + " MHz" +
    "\nRSSI: " + String(WiFi.RSSI(index)) + " dBm" +
    "\nQualidade: " + String(signalPercent(WiFi.RSSI(index))) + "%" +
    "\nSeguranca: " + securityName(WiFi.encryptionType(index))
  );
}
