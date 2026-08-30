#include "wifi_diagnostic.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include "ui.h"
static long tcpTest(IPAddress ip,uint16_t port,uint32_t timeout){ WiFiClient c; uint32_t t=millis(); bool ok=c.connect(ip,port,timeout); long d=millis()-t; c.stop(); return ok?d:-1; }
void runWifiDiagnostic(const String& ssid,const String& password){
  tft.fillScreen(TFT_BLACK); uiHeader("Diagnóstico WiFi"); tft.setCursor(5,38); tft.setTextColor(TFT_WHITE); tft.print("Conectando...");
  WiFi.mode(WIFI_STA); WiFi.begin(ssid.c_str(),password.c_str()); uint32_t start=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-start<10000) delay(100);
  if(WiFi.status()!=WL_CONNECTED){ uiMessage("Falha ao conectar. Verifique SSID e senha e tente novamente."); return; }
  long gw=tcpTest(WiFi.gatewayIP(),80,1500); long dns=tcpTest(IPAddress(8,8,8,8),53,1500);
  uiMessage("Rede: "+ssid+"\nGateway: "+WiFi.gatewayIP().toString()+"\nTCP gateway: "+String(gw<0?"FAIL":String(gw)+" ms")+"\nTCP 8.8.8.8: "+String(dns<0?"FAIL":String(dns)+" ms")+"\nIP: "+WiFi.localIP().toString());
  WiFi.disconnect(true);
}
