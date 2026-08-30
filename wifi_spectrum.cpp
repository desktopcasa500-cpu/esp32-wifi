#include "wifi_spectrum.h"
#include <WiFi.h>
#include "ui.h"
void showWifiSpectrum(){int peak[14]={};WiFi.mode(WIFI_STA);int n=WiFi.scanNetworks(false,true);for(int i=0;i<n;i++){int ch=WiFi.channel(i);if(ch>=1&&ch<=13){int v=constrain(map(WiFi.RSSI(i),-100,-30,0,100),0,100);peak[ch]=max(peak[ch],v);}}tft.fillScreen(TFT_BLACK);uiHeader("2.4 GHz Spectrum");for(int ch=1;ch<=13;ch++){int h=map(peak[ch],0,100,0,150);tft.fillRect(8+(ch-1)*23,195-h,17,h,TFT_CYAN);tft.setTextColor(TFT_WHITE);tft.drawString(String(ch),11+(ch-1)*23,200);}uiFooter();WiFi.scanDelete();}
