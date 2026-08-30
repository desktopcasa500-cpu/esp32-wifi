#include "wifi_hidden_detect.h"
#include <WiFi.h>
#include "ui.h"

void showHiddenNetworks() {
  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();

  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / HIDDEN");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Passive scan...", 8, 50, 1);

  const int total = WiFi.scanNetworks(false, true);
  if (total < 0) {
    WiFi.scanDelete();
    uiMessage("Falha ao iniciar o scan.", "WIFI / HIDDEN");
    return;
  }

  int found = 0;
  for (int i = 0; i < total && found < 7; ++i) {
    if (WiFi.SSID(i).length() != 0) continue;
    const int y = 43 + found * 22;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(WiFi.BSSIDstr(i), 8, y, 1);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(String(WiFi.RSSI(i)) + " dBm  CH" + String(WiFi.channel(i)), 175, y, 1);
    ++found;
  }

  WiFi.scanDelete();
  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString(found ? String(found) + " hidden networks" : "No hidden SSIDs observed", 8, 199, 1);
  uiFooter();
  while (uiReadInput() == BE_NONE) delay(8);
}
