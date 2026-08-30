#include "bt_scanner.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include "ui.h"
#include "buttons.h"
#include "touch.h"
#include "settings.h"

static BLEScan* scanner = nullptr;
static String lastAddress;
static String lastName;

static String deviceName(BLEAdvertisedDevice& d) {
  if (!d.haveName()) return "<unknown>";
  String n = d.getName().c_str();
  n.replace("\n", " ");
  return n.length() ? n : "<unknown>";
}

static void drawBleList(BLEScanResults& results, int selected) {
  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / DEVICES");

  const int total = results.getCount();
  const int shown = min(total, 7);
  const int first = min(max(0, selected - 3), max(0, total - shown));

  for (int i = 0; i < shown; ++i) {
    const int index = first + i;
    BLEAdvertisedDevice d = results.getDevice(index);
    const bool active = index == selected;
    const int y = 39 + i * 23;
    const uint16_t bg = active ? 0x2945 : TFT_BLACK;

    if (active) tft.fillRoundRect(5, y - 3, 310, 20, 2, bg);
    tft.setTextColor(active ? TFT_WHITE : 0xD6BA, bg);
    tft.drawString(String(index + 1) + "  " + deviceName(d).substring(0, 18), 12, y);
    tft.setTextColor(TFT_CYAN, bg);
    tft.drawRightString(String(d.getRSSI()) + "dBm", 310, y, 2);
    tft.setTextColor(0x7BEF, bg);
    tft.drawString(String(d.getAddress().toString().c_str()), 22, y + 11);
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawRightString(String(total) + " devices", 310, 196, 2);
  uiFooter();
}

void bleScan(uint32_t seconds) {
  seconds = constrain(seconds, 1UL, 15UL);

  BLEDevice::init("");
  scanner = BLEDevice::getScan();
  scanner->setActiveScan(settings.activeBleScan);
  scanner->setInterval(100);
  scanner->setWindow(80);

  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / SCAN");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Scanning...", 8, 46);
  tft.drawString(String(seconds) + " sec", 8, 68);

  BLEScanResults results = scanner->start(seconds, false);
  const int total = results.getCount();
  if (total == 0) {
    scanner->clearResults();
    uiMessage("Nenhum dispositivo BLE encontrado.\n\nTente novamente.", "BLE / SCAN");
    return;
  }

  int selected = 0;
  bool done = false;
  drawBleList(results, selected);

  while (!done) {
    const ButtonEvent be = buttonsRead();
    const TouchEvent te = touchRead();

    if (be == BE_PREV || te == TOUCH_PREV) {
      selected = (selected + total - 1) % total;
      drawBleList(results, selected);
    } else if (be == BE_NEXT || te == TOUCH_NEXT) {
      selected = (selected + 1) % total;
      drawBleList(results, selected);
    } else if (be == BE_SELECT || te == TOUCH_SELECT) {
      BLEAdvertisedDevice d = results.getDevice(selected);
      lastAddress = d.getAddress().toString().c_str();
      lastName = deviceName(d);
      uiMessage("Nome: " + lastName + "\nEndereco: " + lastAddress + "\nRSSI: " + String(d.getRSSI()) + " dBm\n\nAlvo salvo para GATT e Proximity.", "BLE / TARGET");
      drawBleList(results, selected);
    } else if (be == BE_BACK || te == TOUCH_BACK) {
      done = true;
    }
    delay(8);
  }

  scanner->clearResults();
  uiToast("BLE scan encerrado");
}

String bleLastSelectedAddress() { return lastAddress; }
String bleLastSelectedName() { return lastName; }
bool bleHasSelection() { return lastAddress.length() > 0; }
