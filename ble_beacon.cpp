#include "ble_beacon.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <string>
#include "ui.h"
#include "buttons.h"
#include "touch.h"

static String classify(const BLEAdvertisedDevice& d) {
  if (!d.haveManufacturerData()) return "BLE";
  const std::string m = d.getManufacturerData();
  if (m.size() >= 25 && (uint8_t)m[0] == 0x4C && (uint8_t)m[1] == 0x00 &&
      (uint8_t)m[2] == 0x02 && (uint8_t)m[3] == 0x15) return "iBeacon";
  if (m.size() >= 3 && (uint8_t)m[0] == 0xAA && (uint8_t)m[1] == 0xFE) {
    const uint8_t frame = (uint8_t)m[2];
    if (frame == 0x10) return "Eddy URL";
    if (frame == 0x20) return "Eddy UID";
    if (frame == 0x00) return "Eddy TLM";
    return "Eddystone";
  }
  if (m.size() >= 4 && (uint8_t)m[0] == 0xFF && (uint8_t)m[1] == 0xFF) return "Vendor";
  return "Manufacturer";
}

static String hexPart(const std::string& m, size_t start, size_t length) {
  static const char* hex = "0123456789ABCDEF";
  String out;
  const size_t end = min(m.size(), start + length);
  for (size_t i = start; i < end; ++i) {
    if (out.length()) out += ' ';
    const uint8_t b = (uint8_t)m[i];
    out += hex[b >> 4]; out += hex[b & 0x0F];
  }
  return out;
}

void beaconDetect(uint32_t seconds) {
  seconds = constrain(seconds, 1UL, 15UL);
  BLEDevice::init("");
  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);

  BLEScanResults* results = scan->start(seconds, false);
  const int total = results ? results->getCount() : 0;
  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / BEACONS");

  int shown = 0;
  for (int i = 0; results && i < total && shown < 7; ++i) {
    BLEAdvertisedDevice d = results->getDevice(i);
    const String type = classify(d);
    if (type == "BLE") continue;
    const int y = 40 + shown * 23;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(type.substring(0, 14), 8, y);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawRightString(String(d.getRSSI()) + "dBm", 310, y);
    tft.setTextColor(0x7BEF, TFT_BLACK);
    if (d.haveManufacturerData()) {
      const std::string m = d.getManufacturerData();
      tft.drawString(hexPart(m, 0, 10), 18, y + 11);
    }
    ++shown;
  }

  if (!shown) tft.drawString("Nenhum beacon reconhecido.", 8, 52);
  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawRightString(String(total) + " devices", 310, 196);
  uiFooter();
  scan->clearResults();

  while (true) {
    const ButtonEvent b = buttonsRead();
    const TouchEvent t = touchRead();
    if (b == BE_BACK || t == TOUCH_BACK || b == BE_SELECT || t == TOUCH_SELECT) break;
    delay(10);
  }
}
