#include "wifi_signal_meter.h"
#include <WiFi.h>
#include "ui.h"
void showSignalMeter(const String& ssid,const String& password){ WiFi.begin(ssid.c_str(),password.c_str());uint32_t t=millis();while(WiFi.status()!=WL_CONNECTED&&millis()-t<10000)delay(100);if(WiFi.status()!=WL_CONNECTED){uiMessage("Não foi possível conectar.");return;}tft.fillScreen(TFT_BLACK);uiHeader("Signal Meter");for(int i=0;i<60;i++){int r=WiFi.RSSI();int w=constrain(map(r,-100,-30,0,280),0,280);tft.fillRect(10,70, w,20,TFT_CYAN);tft.setTextColor(TFT_WHITE);tft.drawString(String(r)+" dBm",10,105);delay(250);}WiFi.disconnect(true);}
