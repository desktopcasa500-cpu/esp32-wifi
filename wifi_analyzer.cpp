#include "wifi_analyzer.h"
#include <WiFi.h>
#include "ui.h"
void wifiScan(bool showHidden){
  WiFi.mode(WIFI_STA); WiFi.disconnect(false); delay(100);
  int n=WiFi.scanNetworks(false,showHidden);
  tft.fillScreen(TFT_BLACK); uiHeader("WiFi Analyzer");
  if(n<=0){ uiMessage("Nenhuma rede encontrada."); return; }
  int shown=min(n,7);
  for(int i=0;i<shown;i++){
    String ssid=WiFi.SSID(i); if(ssid.length()==0) ssid="<hidden>";
    tft.setTextColor(TFT_WHITE); tft.drawString(String(i+1)+" "+ssid.substring(0,19),5,35+i*24);
    tft.setTextColor(TFT_CYAN); tft.drawString(String(WiFi.RSSI(i))+" dBm  CH "+String(WiFi.channel(i)),220,35+i*24);
  }
  uiFooter(); WiFi.scanDelete();
}
void wifiPrintDetails(int index){
  if(index<0 || index>=WiFi.scanComplete()) return;
  uiMessage("SSID: "+WiFi.SSID(index)+"\nBSSID: "+WiFi.BSSIDstr(index)+"\nCanal: "+String(WiFi.channel(index))+"\nRSSI: "+String(WiFi.RSSI(index))+" dBm\nSegurança: "+String(WiFi.encryptionType(index)));
}
