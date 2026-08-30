#include "wifi_channel_scan.h"
#include <WiFi.h>
#include "ui.h"
void showChannelScan(){ int c[14]={}; WiFi.mode(WIFI_STA); int n=WiFi.scanNetworks(false,true); for(int i=0;i<n;i++){int ch=WiFi.channel(i);if(ch>=1&&ch<=13)c[ch]++;} tft.fillScreen(TFT_BLACK);uiHeader("2.4 GHz Channels"); for(int ch=1;ch<=13;ch++){int h=min(150,c[ch]*15); tft.drawRect(8+(ch-1)*23,195-h,17,h,TFT_CYAN);tft.setTextColor(TFT_WHITE);tft.drawString(String(ch),11+(ch-1)*23,200);}uiFooter();WiFi.scanDelete(); }
