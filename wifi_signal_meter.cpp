#include "wifi_signal_meter.h"
#include <WiFi.h>
#include "ui.h"
#include "buttons.h"
#include "touch.h"

static int quality(int rssi) {
  return constrain(map(rssi, -95, -35, 0, 100), 0, 100);
}

void showSignalMeter(const String& ssid, const String& bssid, uint32_t durationMs) {
  if (!ssid.length() && !bssid.length()) {
    uiMessage("Nenhuma rede selecionada.\n\nAbra o WiFi Analyzer e selecione uma rede primeiro.", "WIFI / SIGNAL");
    return;
  }

  int16_t history[60] = {};
  int historyCount = 0;
  int minRssi = 127;
  int maxRssi = -127;
  uint32_t samples = 0;
  const uint32_t endAt = millis() + durationMs;

  while (millis() < endAt) {
    WiFi.mode(WIFI_STA);
    WiFi.scanDelete();
    const int total = WiFi.scanNetworks(false, true);
    int current = -127;

    for (int i = 0; i < total; ++i) {
      const String seenBssid = WiFi.BSSIDstr(i);
      const String seenSsid = WiFi.SSID(i);
      if ((bssid.length() && seenBssid.equalsIgnoreCase(bssid)) ||
          (!bssid.length() && seenSsid == ssid)) {
        current = WiFi.RSSI(i);
        break;
      }
    }

    WiFi.scanDelete();

    if (current > -127) {
      if (historyCount < 60) history[historyCount++] = current;
      else {
        for (int i = 1; i < 60; ++i) history[i - 1] = history[i];
        history[59] = current;
      }
      minRssi = min(minRssi, current);
      maxRssi = max(maxRssi, current);
      ++samples;
    }

    tft.fillScreen(TFT_BLACK);
    uiHeader("WIFI / SIGNAL", "LIVE");
    tft.setTextColor(0xD6BA, TFT_BLACK);
    tft.drawString(ssid.length() ? ssid.substring(0, 20) : "<hidden>", 8, 38);

    const int left = 12, top = 62, width = 296, height = 90;
    tft.drawRect(left, top, width, height, 0x39C7);

    if (historyCount > 1) {
      for (int i = 1; i < historyCount; ++i) {
        const int x1 = map(i - 1, 0, max(1, historyCount - 1), left + 2, left + width - 3);
        const int x2 = map(i, 0, max(1, historyCount - 1), left + 2, left + width - 3);
        const int y1 = map(constrain(history[i - 1], -100, -30), -100, -30, top + height - 3, top + 3);
        const int y2 = map(constrain(history[i], -100, -30), -100, -30, top + height - 3, top + 3);
        tft.drawLine(x1, y1, x2, y2, TFT_CYAN);
      }
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    if (current > -127) {
      tft.drawString(String(current) + " dBm", 12, 166);
      tft.drawRightString(String(quality(current)) + "%", 307, 166, 2);
    } else {
      tft.drawString("not seen", 12, 166);
    }

    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.drawString("min " + String(samples ? minRssi : 0) + "  max " + String(samples ? maxRssi : 0), 12, 190);
    tft.drawRightString(String(samples) + " samples", 307, 190, 2);
    uiFooter();

    const ButtonEvent be = buttonsRead();
    const TouchEvent te = touchRead();
    if (be == BE_BACK || te == TOUCH_BACK) break;
    delay(1200);
  }

  uiMessage(
    "Target: " + (ssid.length() ? ssid : bssid) +
    "\nSamples: " + String(samples) +
    "\nMin RSSI: " + String(samples ? minRssi : 0) + " dBm" +
    "\nMax RSSI: " + String(samples ? maxRssi : 0) + " dBm",
    "WIFI / SIGNAL RESULT"
  );
}
