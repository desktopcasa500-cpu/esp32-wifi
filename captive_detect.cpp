#include "captive_detect.h"
#include <WiFi.h>
#include <HTTPClient.h>
bool detectCaptivePortal(const String& ssid,const String& password,String& report){
  WiFi.mode(WIFI_STA); WiFi.begin(ssid.c_str(),password.c_str()); uint32_t t=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-t<10000) delay(100);
  if(WiFi.status()!=WL_CONNECTED){ report="Falha de conexão"; return false; }
  HTTPClient http; http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); http.begin("http://connectivitycheck.gstatic.com/generate_204"); int code=http.GET(); String url=http.getLocation(); http.end(); WiFi.disconnect(true);
  if(code==204){report="Sem portal cativo detectado (HTTP 204)."; return false;}
  report="Possível portal cativo ou interceptação HTTP. Código="+String(code); if(url.length()) report+="\nDestino="+url; return true;
}
