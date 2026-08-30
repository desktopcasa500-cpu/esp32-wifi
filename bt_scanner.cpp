#include "bt_scanner.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include "ui.h"
static BLEScan* scanner=nullptr;
void bleScan(uint32_t seconds){
  BLEDevice::init(""); scanner=BLEDevice::getScan(); scanner->setActiveScan(true); BLEScanResults results=scanner->start(seconds,false);
  tft.fillScreen(TFT_BLACK); uiHeader("BLE Scanner"); int shown=min(results.getCount(),7);
  for(int i=0;i<shown;i++){ BLEAdvertisedDevice d=results.getDevice(i); String name=d.haveName()?d.getName().c_str():"<unknown>"; tft.setTextColor(TFT_WHITE); tft.drawString(name.substring(0,18),4,36+i*24); tft.setTextColor(TFT_CYAN); tft.drawString(String(d.getRSSI())+" dBm",225,36+i*24); }
  uiFooter(); scanner->clearResults();
}
