#include "ble_advertiser.h"
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <string>
#include "settings.h"

namespace {
bool active = false;
BLEAdvertising* advertising = nullptr;

String profileName(uint8_t profile) {
  switch (profile) {
    case 1: return "ESP32 Environmental";
    case 2: return "ESP32 Heart Monitor";
    case 3: return "ESP32 Location";
    case 4: return "ESP32 Battery";
    case 5: return "ESP32 HID Demo";
    case 6: return "ESP32 Clock";
    case 7: return "ESP32 Ruuvi Demo";
    case 8: return "ESP32 iBeacon";
    case 9: return settings.deviceName;
    default: return settings.deviceName;
  }
}

std::string ibeaconPayload() {
  static const uint8_t bytes[] = {
    0x4C, 0x00, 0x02, 0x15,
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC,
    0x00, 0x01, 0x00, 0x01,
    0xC5
  };
  return std::string(reinterpret_cast<const char*>(bytes), sizeof(bytes));
}
}

void bleAdvertiserStart(uint8_t profile, uint16_t intervalMs) {
  bleAdvertiserStop();
  intervalMs = constrain(intervalMs, 50, 1000);

  BLEDevice::init(settings.deviceName.c_str());
  advertising = BLEDevice::getAdvertising();
  if (advertising == nullptr) return;

  advertising->stop();
  BLEAdvertisementData data;
  data.setFlags(0x06);

  if (profile == 8) {
    data.setManufacturerData(String(ibeaconPayload().c_str()));
    data.setName("ESP32 iBeacon");
  } else {
    data.setName(profileName(profile));
  }

  advertising->setAdvertisementData(data);
  const uint16_t units = max<uint16_t>(80, static_cast<uint16_t>(intervalMs / 0.625f));
  advertising->setMinInterval(units);
  advertising->setMaxInterval(units);
  advertising->setScanResponse(false);
  advertising->start();
  active = true;
}

void bleAdvertiserStop() {
  if (advertising != nullptr) advertising->stop();
  active = false;
}
