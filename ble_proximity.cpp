#include "ble_proximity.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include "ui.h"
void bleProximityMonitor(const String& address,uint32_t durationMs){
  BLEDevice::init(""); BLEScan* scan=BLEDevice::getScan(); scan->setActiveScan(true);
  int minR=127,maxR=-127,count=0; uint32_t start=millis();
  while(millis()-start<durationMs){ BLEScanResults r=scan->start(1,false); for(int i=0;i<r.getCount();i++){ auto d=r.getDevice(i); if(String(d.getAddress().toString().c_str()).equalsIgnoreCase(address)){int v=d.getRSSI();minR=min(minR,v);maxR=max(maxR,v);count++; tft.fillScreen(TFT_BLACK);uiHeader("BLE Proximity");tft.setTextColor(TFT_WHITE);tft.drawString("Target: "+address,5,40);tft.drawString("RSSI: "+String(v)+" dBm",5,70);tft.drawString("Samples: "+String(count),5,100);tft.drawString("Min/Max: "+String(minR)+" / "+String(maxR),5,130);uiFooter();}}
    scan->clearResults();
  }
  uiMessage("Monitor concluído\nAmostras: "+String(count)+"\nRSSI mínimo: "+String(minR)+" dBm\nRSSI máximo: "+String(maxR)+" dBm");
}
