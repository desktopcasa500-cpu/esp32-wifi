#include "ble_beacon.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include "ui.h"
static String classify(BLEAdvertisedDevice& d){
  if(!d.haveManufacturerData()) return "BLE";
  std::string m=d.getManufacturerData();
  if(m.size()>=25 && (uint8_t)m[0]==0x4c && (uint8_t)m[1]==0x00 && (uint8_t)m[2]==0x02 && (uint8_t)m[3]==0x15) return "iBeacon";
  if(m.size()>=3 && (uint8_t)m[0]==0xaa && (uint8_t)m[1]==0xfe) return "Eddystone";
  return "Manufacturer data";
}
void beaconDetect(uint32_t seconds){
  BLEDevice::init("");BLEScan* s=BLEDevice::getScan();s->setActiveScan(true);BLEScanResults r=s->start(seconds,false);
  tft.fillScreen(TFT_BLACK);uiHeader("Beacon Detector");int y=38,shown=0;
  for(int i=0;i<r.getCount()&&shown<7;i++){auto d=r.getDevice(i);String type=classify(d);if(type!="BLE"){tft.setTextColor(TFT_WHITE);tft.drawString(type,4,y);tft.setTextColor(TFT_CYAN);tft.drawString(String(d.getRSSI())+" dBm",220,y);y+=25;shown++;}}
  if(!shown)tft.drawString("Nenhum beacon reconhecido.",5,50);uiFooter();s->clearResults();
}
