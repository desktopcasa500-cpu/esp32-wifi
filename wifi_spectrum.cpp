#include "wifi_spectrum.h"
#include <WiFi.h>
#include "ui.h"

void showWifiSpectrum() {
  uint8_t level[14] = {0};

  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / SPECTRUM", "2.4G");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Scanning...", 8, 49, 1);

  const int total = WiFi.scanNetworks(false, true);
  if (total < 0) {
    WiFi.scanDelete();
    uiMessage("Nao foi possivel iniciar o scan.", "WIFI / SPECTRUM");
    return;
  }

  for (int i = 0; i < total; ++i) {
    const int ch = WiFi.channel(i);
    if (ch < 1 || ch > 13) continue;
    const int value = constrain(map(WiFi.RSSI(i), -100, -30, 0, 100), 0, 100);
    if (value > level[ch]) level[ch] = value;
  }
  WiFi.scanDelete();

  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / SPECTRUM", "2.4G");
  const int base = 182;
  const int top = 51;
  for (int ch = 1; ch <= 13; ++ch) {
    const int x = 6 + (ch - 1) * 24;
    const int h = map(level[ch], 0, 100, 0, base - top);
    if (level[ch] == 0) tft.drawRect(x, top, 18, base - top, 0x39C7);
    else tft.fillRect(x, base - h, 18, h, TFT_CYAN);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString(String(ch), x + 9, 188, 1);
  }
  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString("Peak signal per channel", 8, 202, 1);
  uiFooter();
  while (uiReadInput() == BE_NONE) delay(8);
}
