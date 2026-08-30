#include "wifi_signal_meter.h"
#include <WiFi.h>
#include "ui.h"

namespace {
int quality(int rssi) {
  return constrain(map(rssi, -95, -35, 0, 100), 0, 100);
}
}

void showSignalMeter(const String& ssid, const String& bssid, uint32_t durationMs) {
  if (!ssid.length() && !bssid.length()) {
    uiMessage("Nenhuma rede selecionada.", "WIFI / SIGNAL");
    return;
  }

  int16_t history[60] = {0};
  uint8_t count = 0;
  int minRssi = 127;
  int maxRssi = -127;
  int current = -127;
  uint32_t samples = 0;
  const uint32_t started = millis();

  while ((uint32_t)(millis() - started) < durationMs) {
    if (uiReadInput() == BE_BACK) break;

    WiFi.mode(WIFI_STA);
    WiFi.scanDelete();
    const int found = WiFi.scanNetworks(false, true);
    current = -127;

    for (int i = 0; i < found; ++i) {
      const String foundBssid = WiFi.BSSIDstr(i);
      const String foundSsid = WiFi.SSID(i);
      if ((bssid.length() && foundBssid.equalsIgnoreCase(bssid)) ||
          (!bssid.length() && foundSsid == ssid)) {
        current = WiFi.RSSI(i);
        break;
      }
    }
    WiFi.scanDelete();

    if (current > -127) {
      if (count < 60) history[count++] = static_cast<int16_t>(current);
      else {
        memmove(history, history + 1, sizeof(history) - sizeof(history[0]));
        history[59] = static_cast<int16_t>(current);
      }
      minRssi = min(minRssi, current);
      maxRssi = max(maxRssi, current);
      ++samples;
    }

    tft.fillScreen(TFT_BLACK);
    uiHeader("WIFI / SIGNAL", "LIVE");
    tft.setTextColor(0xD6BA, TFT_BLACK);
    tft.drawString(ssid.length() ? ssid.substring(0, 20) : bssid.substring(0, 20), 8, 39, 1);

    const int left = 12, top = 61, width = 296, height = 91;
    tft.drawRect(left, top, width, height, 0x39C7);
    for (uint8_t i = 1; i < count; ++i) {
      const int x1 = map(i - 1, 0, max(1, (int)count - 1), left + 2, left + width - 3);
      const int x2 = map(i, 0, max(1, (int)count - 1), left + 2, left + width - 3);
      const int y1 = map(constrain((int)history[i - 1], -100, -30), -100, -30, top + height - 3, top + 3);
      const int y2 = map(constrain((int)history[i], -100, -30), -100, -30, top + height - 3, top + 3);
      tft.drawLine(x1, y1, x2, y2, TFT_CYAN);
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    if (current > -127) {
      tft.drawString(String(current) + " dBm", 12, 166, 2);
      tft.drawRightString(String(quality(current)) + "%", 307, 166, 2);
    } else {
      tft.drawString("not seen", 12, 166, 1);
    }
    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.drawString("min " + String(samples ? minRssi : 0) + "  max " + String(samples ? maxRssi : 0), 12, 190, 1);
    uiFooter();
    delay(900);
  }

  uiMessage("Target: " + (ssid.length() ? ssid : bssid) +
            "\nSamples: " + String(samples) +
            "\nMin: " + String(samples ? minRssi : 0) + " dBm" +
            "\nMax: " + String(samples ? maxRssi : 0) + " dBm", "WIFI / RESULT");
}
