#include "wifi_channel_scan.h"
#include <WiFi.h>
#include "ui.h"

void showChannelScan() {
  int count[14] = {};

  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  const int n = WiFi.scanNetworks(false, true);

  for (int i = 0; i < n; ++i) {
    const int ch = WiFi.channel(i);
    if (ch >= 1 && ch <= 13) ++count[ch];
  }

  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / CHANNEL LOAD");

  int maxCount = 1;
  for (int ch = 1; ch <= 13; ++ch) maxCount = max(maxCount, count[ch]);

  const int baseY = 183;
  const int chartTop = 55;
  const int barW = 17;
  const int gap = 7;

  for (int ch = 1; ch <= 13; ++ch) {
    const int x = 7 + (ch - 1) * (barW + gap);
    const int h = map(count[ch], 0, maxCount, 0, baseY - chartTop);

    if (count[ch] == 0) tft.drawRect(x, chartTop, barW, baseY - chartTop, 0x39C7);
    else tft.fillRect(x, baseY - h, barW, h, TFT_CYAN);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString(String(ch), x + barW / 2, 188, 1);
    tft.setTextColor(0x7BEF, TFT_BLACK);
    if (count[ch] > 0) tft.drawCentreString(String(count[ch]), x + barW / 2, max(chartTop, baseY - h - 12), 1);
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString(String(n) + " networks", 8, 38);
  tft.drawString("1 / 6 / 11  recommended", 8, 202);
  uiFooter();
  WiFi.scanDelete();
}
