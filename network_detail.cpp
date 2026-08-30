#include "network_detail.h"
#include <WiFi.h>
#include "ui.h"
void showNetworkDetail(int index){int n=WiFi.scanComplete();if(index<0||index>=n){uiMessage("Nenhuma rede selecionada.");return;}String s=WiFi.SSID(index);if(!s.length())s="<hidden>";String out="SSID: "+s+"\nBSSID: "+WiFi.BSSIDstr(index)+"\nCanal: "+String(WiFi.channel(index))+"\nFrequência: "+String(2407+WiFi.channel(index)*5)+" MHz\nRSSI: "+String(WiFi.RSSI(index))+" dBm\nSegurança: "+String((int)WiFi.encryptionType(index));uiMessage(out);}
