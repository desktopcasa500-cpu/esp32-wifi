#include "bt_scanner.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include "ui.h"
#include "settings.h"

namespace {
BLEScan* scanner = nullptr;
String lastAddress;
String lastName;

String safeName(BLEAdvertisedDevice& device) {
  if (!device.haveName()) return "<unknown>";
  String name = String(device.getName().c_str());
  name.replace('\r', ' ');
  name.replace('\n', ' ');
  if (!name.length()) name = "<unknown>";
  return name;
}

void drawResults(BLEScanResults& results, int selected) {
  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / SCANNER", String(results.getCount()) + " DEV");

  const int total = results.getCount();
  const int visible = min(total, 7);
  const int first = min(max(0, selected - 3), max(0, total - visible));

  for (int row = 0; row < visible; ++row) {
    const int index = first + row;
    BLEAdvertisedDevice device = results.getDevice(index);
    const bool active = index == selected;
    const uint16_t bg = active ? 0x2945 : TFT_BLACK;
    const int y = 39 + row * 24;

    if (active) tft.fillRoundRect(5, y - 3, 310, 21, 3, bg);
    tft.setTextColor(active ? TFT_WHITE : 0xD6BA, bg);
    tft.drawString(String(index + 1) + "  " + safeName(device).substring(0, 17), 12, y, 1);
    tft.setTextColor(TFT_CYAN, bg);
    tft.drawRightString(String(device.getRSSI()) + "dBm", 309, y, 1);
    tft.setTextColor(0x7BEF, bg);
    tft.drawString(device.getAddress().toString().c_str(), 23, y + 11, 1);
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString(String(selected + 1) + "/" + String(total), 8, 195, 1);
  uiFooter();
}
}

void bleScan(uint32_t seconds) {
  seconds = constrain(seconds, 1UL, 15UL);
  BLEDevice::init("");
  scanner = BLEDevice::getScan();
  scanner->setActiveScan(settings.activeBleScan);
  scanner->setInterval(100);
  scanner->setWindow(80);

  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / SCANNER");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Scanning...", 8, 50, 1);

  BLEScanResults results = scanner->start(seconds, false);
  const int total = results.getCount();
  if (total <= 0) {
    scanner->clearResults();
    uiMessage("Nenhum dispositivo BLE encontrado.", "BLE / SCANNER");
    return;
  }

  int selected = 0;
  drawResults(results, selected);
  while (true) {
    const ButtonEvent ev = uiReadInput();
    if (ev == BE_PREV) {
      selected = (selected + total - 1) % total;
      drawResults(results, selected);
    } else if (ev == BE_NEXT) {
      selected = (selected + 1) % total;
      drawResults(results, selected);
    } else if (ev == BE_SELECT) {
      BLEAdvertisedDevice device = results.getDevice(selected);
      lastAddress = device.getAddress().toString().c_str();
      lastName = safeName(device);
      uiMessage("Name: " + lastName +
                "\nAddress: " + lastAddress +
                "\nRSSI: " + String(device.getRSSI()) + " dBm",
                "BLE / TARGET");
      drawResults(results, selected);
    } else if (ev == BE_BACK) {
      break;
    }
    delay(8);
  }

  scanner->clearResults();
  uiToast("BLE scan stopped", 500);
}

String bleLastSelectedAddress() { return lastAddress; }
String bleLastSelectedName() { return lastName; }
bool bleHasSelection() { return lastAddress.length() == 17; }
