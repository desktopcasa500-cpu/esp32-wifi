#include "ble_inspector.h"
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include "ui.h"
void bleInspectDevice(const String& address){
  BLEAddress addr(address.c_str());
  BLEClient* client=BLEDevice::createClient();
  tft.fillScreen(TFT_BLACK); uiHeader("GATT Inspector");
  if(!client->connect(addr)){ uiMessage("Falha ao conectar ao dispositivo BLE.\nUse somente dispositivos autorizados."); delete client; return; }
  auto* services=client->getServices();
  String out="Conectado\nServiços: "+String(services?services->size():0)+"\n\n";
  if(services){ for(auto const& p:*services){ out+="S: "+String(p.first.c_str())+"\n"; auto* chars=p.second->getCharacteristics(); if(chars){ for(auto const& c:*chars) out+="  C: "+String(c.first.c_str())+"\n"; } } }
  client->disconnect(); delete client; uiMessage(out);
}
