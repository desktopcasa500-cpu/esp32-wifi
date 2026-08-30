#include "wifi_channel_scan.h"
#include <WiFi.h>
#include "ui.h"

void showChannelScan() {
  int count[14] = {0};
  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();

  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / CHANNELS");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Scanning 2.4 GHz...", 8, 50, 1);

  const int total = WiFi.scanNetworks(false, true);
  if (total < 0) {
    WiFi.scanDelete();
    uiMessage("Falha ao iniciar o scan.", "WIFI / CHANNELS");
    return;
  }

  for (int i = 0; i < total; ++i) {
    const int ch = WiFi.channel(i);
    if (ch >= 1 && ch <= 13) ++count[ch];
  }
  WiFi.scanDelete();

  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / CHANNELS", String(total) + " NET");
  int maxCount = 1;
  for (int ch = 1; ch <= 13; ++ch) maxCount = max(maxCount, count[ch]);

  const int left = 6;
  const int top = 50;
  const int base = 183;
  const int barW = 18;
  const int gap = 5;
  for (int ch = 1; ch <= 13; ++ch) {
    const int x = left + (ch - 1) * (barW + gap);
    const int h = map(count[ch], 0, maxCount, 0, base - top);
    if (count[ch] == 0) tft.drawRect(x, top, barW, base - top, 0x39C7);
    else tft.fillRect(x, base - h, barW, h, TFT_CYAN);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString(String(ch), x + barW / 2, 188, 1);
    if (count[ch]) {
      tft.setTextColor(0x7BEF, TFT_BLACK);
      tft.drawCentreString(String(count[ch]), x + barW / 2, max(top, base - h - 10), 1);
    }
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString("Menor ocupacao = melhor candidato", 8, 201, 1);
  uiFooter();

  while (uiReadInput() == BE_NONE) delay(8);
}
