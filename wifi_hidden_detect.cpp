#include "wifi_hidden_detect.h"
#include <WiFi.h>
#include "ui.h"
void showHiddenNetworks(){WiFi.mode(WIFI_STA);int n=WiFi.scanNetworks(false,true);int found=0;tft.fillScreen(TFT_BLACK);uiHeader("Hidden Networks");for(int i=0;i<n&&found<7;i++){if(WiFi.SSID(i).length()==0){tft.setTextColor(TFT_WHITE);tft.drawString("BSSID "+WiFi.BSSIDstr(i),4,38+found*25);tft.setTextColor(TFT_CYAN);tft.drawString(String(WiFi.RSSI(i))+" dBm CH "+String(WiFi.channel(i)),4,53+found*25);found++;}}if(!found)tft.drawString("Nenhuma rede oculta vista.",5,45);uiFooter();WiFi.scanDelete();}
