#include "ui.h"
TFT_eSPI tft=TFT_eSPI();
void uiBegin(){ tft.init(); tft.setRotation(1); tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_CYAN,TFT_BLACK); tft.setTextFont(2); }
void uiHeader(const String& title){ tft.fillRect(0,0,320,28,TFT_DARKGREY); tft.setTextColor(TFT_WHITE,TFT_DARKGREY); tft.setCursor(8,7); tft.print(title); }
void uiFooter(){ tft.fillRect(0,215,320,25,TFT_DARKGREY); tft.setTextColor(TFT_WHITE,TFT_DARKGREY); tft.drawString("PREV       SEL       NEXT",65,222); }
void uiMessage(const String& text){ tft.fillScreen(TFT_BLACK); uiHeader("ESP32 WiFi Toolkit"); tft.setTextColor(TFT_WHITE,TFT_BLACK); tft.setCursor(8,45); tft.setTextWrap(true); tft.print(text); uiFooter(); }
void uiMenu(const String items[], size_t count, int selected){ tft.fillScreen(TFT_BLACK); uiHeader("ESP32 WiFi Toolkit"); for(size_t i=0;i<count;i++){ int y=38+i*28; if((int)i==selected) tft.fillRect(4,y-3,312,24,TFT_DARKCYAN); tft.setTextColor(TFT_WHITE); tft.drawString(items[i],12,y); } uiFooter(); }
