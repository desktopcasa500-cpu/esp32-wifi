#include "network_detail.h"
#include <WiFi.h>
#include "ui.h"

namespace {
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
}

void showNetworkDetail(int index) {
  const int total = WiFi.scanComplete();
  if (total <= 0 || index < 0 || index >= total) {
    uiMessage("Nenhum resultado de scan disponivel.", "WIFI / DETAIL");
    return;
  }

  const int channel = WiFi.channel(index);
  const int frequency = channel == 14 ? 2484 : 2407 + channel * 5;
  String ssid = WiFi.SSID(index);
  if (!ssid.length()) ssid = "<hidden>";

  String report;
  report.reserve(240);
  report += "SSID: "; report += ssid;
  report += "\nBSSID: "; report += WiFi.BSSIDstr(index);
  report += "\nChannel: "; report += String(channel);
  report += "\nFrequency: "; report += String(frequency); report += " MHz";
  report += "\nRSSI: "; report += String(WiFi.RSSI(index)); report += " dBm";
  report += "\nSecurity: "; report += securityName(WiFi.encryptionType(index));
  uiMessage(report, "WIFI / DETAIL");
}
