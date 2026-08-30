#include "ble_advertiser.h"
#include "settings.h"
#include <BLEDevice.h>
#include <BLEAdvertising.h>

static bool active = false;

void bleAdvertiserStart(uint8_t profile, uint16_t intervalMs) {
  intervalMs = constrain(intervalMs, 50, 1000);
  BLEDevice::init(settings.deviceName.c_str());
  BLEAdvertising* advertising = BLEDevice::getAdvertising();

  BLEAdvertisementData data;
  String name;
  switch (profile) {
    case 1: name = "ESP32 Environmental"; break;
    case 2: name = "ESP32 Heart Monitor"; break;
    case 3: name = "ESP32 Location"; break;
    case 4: name = "ESP32 Battery"; break;
    case 5: name = "ESP32 HID Demo"; break;
    case 6: name = "ESP32 Clock"; break;
    case 7: name = "ESP32 Sensor"; break;
    case 8: name = "ESP32 iBeacon Demo"; break;
    case 9: name = settings.deviceName; break;
    default: name = settings.deviceName; break;
  }

  data.setName(name.c_str());
  advertising->setAdvertisementData(data);

  const uint16_t units = max<uint16_t>(80, (uint16_t)(intervalMs / 0.625f));
  advertising->setMinInterval(units);
  advertising->setMaxInterval(units);
  advertising->start();
  active = true;
}

void bleAdvertiserStop() {
  if (!active) return;
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  if (advertising) advertising->stop();
  active = false;
}
