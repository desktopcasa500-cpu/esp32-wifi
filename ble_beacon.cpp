#include "ble_beacon.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <string>
#include "ui.h"

namespace {
String beaconType(BLEAdvertisedDevice& device) {
  if (!device.haveManufacturerData()) return "BLE";
  const std::string data = device.getManufacturerData();
  if (data.size() >= 4 &&
      static_cast<uint8_t>(data[0]) == 0x4C && static_cast<uint8_t>(data[1]) == 0x00 &&
      static_cast<uint8_t>(data[2]) == 0x02 && static_cast<uint8_t>(data[3]) == 0x15) {
    return "iBeacon";
  }
  if (data.size() >= 3 && static_cast<uint8_t>(data[0]) == 0xAA && static_cast<uint8_t>(data[1]) == 0xFE) {
    switch (static_cast<uint8_t>(data[2])) {
      case 0x10: return "Eddy URL";
      case 0x20: return "Eddy UID";
      case 0x00: return "Eddy TLM";
      default: return "Eddystone";
    }
  }
  return "Manufacturer";
}

String hexPreview(const std::string& data, size_t maxBytes) {
  static const char* HEX = "0123456789ABCDEF";
  String out;
  const size_t count = min(maxBytes, data.size());
  for (size_t i = 0; i < count; ++i) {
    if (i) out += ' ';
    const uint8_t b = static_cast<uint8_t>(data[i]);
    out += HEX[b >> 4];
    out += HEX[b & 0x0F];
  }
  return out;
}
}

void beaconDetect(uint32_t seconds) {
  seconds = constrain(seconds, 1UL, 15UL);
  BLEDevice::init("");
  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);

  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / BEACONS", "SCAN");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Scanning...", 8, 50, 1);

  BLEScanResults results = scan->start(seconds, false);
  const int total = results.getCount();

  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / BEACONS", String(total) + " DEV");
  int shown = 0;

  for (int i = 0; i < total && shown < 6; ++i) {
    BLEAdvertisedDevice device = results.getDevice(i);
    const String type = beaconType(device);
    if (type == "BLE") continue;

    const int y = 39 + shown * 27;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(type.substring(0, 13), 8, y, 1);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawRightString(String(device.getRSSI()) + "dBm", 309, y, 1);
    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.drawString(device.getAddress().toString().c_str(), 8, y + 12, 1);
    if (device.haveManufacturerData()) {
      tft.drawString(hexPreview(device.getManufacturerData(), 7), 178, y + 12, 1);
    }
    ++shown;
  }

  if (shown == 0) {
    tft.setTextColor(0x7BEF, TFT_BLACK);
    tft.drawString("Nenhum beacon reconhecido.", 8, 55, 1);
  }

  uiFooter();
  scan->clearResults();
  while (uiReadInput() == BE_NONE) delay(8);
}
