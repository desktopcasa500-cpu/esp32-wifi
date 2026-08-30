#include "ble_advertiser.h"
#include <BLEDevice.h>
#include <BLEAdvertising.h>
static bool active=false;
void bleAdvertiserStart(uint8_t profile,uint16_t intervalMs){
  BLEDevice::init("ESP32-WiFi-Toolkit");BLEAdvertising* a=BLEDevice::getAdvertising();
  BLEAdvertisementData data; String name;
  switch(profile){case 1:name="ESP32 Environmental Sensor";break;case 2:name="ESP32 Heart Monitor";break;case 3:name="ESP32 Location";break;case 4:name="ESP32 Battery";break;case 5:name="ESP32 HID";break;case 6:name="ESP32 Clock";break;case 7:name="ESP32 Ruuvi";break;case 8:name="ESP32 iBeacon Demo";break;default:name="ESP32 WiFi Toolkit";break;}
  data.setName(name.c_str());a->setAdvertisementData(data);a->setMinInterval(intervalMs/0.625);a->setMaxInterval(intervalMs/0.625);a->start();active=true;
}
void bleAdvertiserStop(){if(active){BLEDevice::getAdvertising()->stop();active=false;}}
