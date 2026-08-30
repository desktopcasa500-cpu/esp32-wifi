#include "bt_scanner.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include "ui.h"
#include "buttons.h"
#include "touch.h"

static BLEScan* scanner = nullptr;

static String deviceName(BLEAdvertisedDevice& d) {
  if (!d.haveName()) return "<unknown>";
  String n = d.getName().c_str();
  n.replace("\n", " ");
  return n.length() ? n : "<unknown>";
}

static void drawBleList(BLEScanResults* results, int selected) {
  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / DEVICES");

  const int total = results ? results->getCount() : 0;
  const int shown = min(total, 7);
  const int first = min(max(0, selected - 3), max(0, total - shown));

  for (int i = 0; i < shown; ++i) {
    const int index = first + i;
    BLEAdvertisedDevice d = results->getDevice(index);
    const bool active = index == selected;
    const int y = 39 + i * 23;

    if (active) tft.fillRoundRect(5, y - 3, 310, 20, 2, 0x2945);
    tft.setTextColor(active ? TFT_WHITE : 0xD6BA, active ? 0x2945 : TFT_BLACK);
    tft.drawString(String(index + 1) + "  " + deviceName(d).substring(0, 18), 12, y);
    tft.setTextColor(TFT_CYAN, active ? 0x2945 : TFT_BLACK);
    tft.drawRightString(String(d.getRSSI()) + "dBm", 310, y);
    tft.setTextColor(0x7BEF, active ? 0x2945 : TFT_BLACK);
    tft.drawString(String(d.getAddress().toString().c_str()), 22, y + 11);
  }

  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawRightString(String(total) + " devices", 310, 196);
  uiFooter();
}

void bleScan(uint32_t seconds) {
  if (seconds < 1) seconds = 1;
  if (seconds > 15) seconds = 15;

  BLEDevice::init("");
  scanner = BLEDevice::getScan();
  scanner->setActiveScan(true);
  scanner->setInterval(100);
  scanner->setWindow(80);

  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / SCAN");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Scanning...", 8, 46);

  BLEScanResults* results = scanner->start(seconds, false);
  const int total = results ? results->getCount() : 0;
  if (total == 0) {
    uiMessage("Nenhum dispositivo BLE encontrado.\n\nTente novamente em uma area com dispositivos proximos.");
    scanner->clearResults();
    return;
  }

  int selected = 0;
  bool done = false;
  drawBleList(results, selected);

  while (!done) {
    ButtonEvent be = buttonsRead();
    TouchEvent te = touchRead();

    if (be == BE_PREV || te == TOUCH_PREV) {
      selected = (selected + total - 1) % total;
      drawBleList(results, selected);
    } else if (be == BE_NEXT || te == TOUCH_NEXT) {
      selected = (selected + 1) % total;
      drawBleList(results, selected);
    } else if (be == BE_SELECT || te == TOUCH_SELECT) {
      BLEAdvertisedDevice d = results->getDevice(selected);
      String info = "Nome: " + deviceName(d) +
                    "\nEndereco: " + String(d.getAddress().toString().c_str()) +
                    "\nRSSI: " + String(d.getRSSI()) + " dBm" +
                    "\nConnectable: " + String(d.isAdvertisingService(BLEUUID((uint16_t)0x1800)) ? "sim" : "desconhecido");
      uiMessage(info, "BLE / DETAIL");
      drawBleList(results, selected);
    } else if (be == BE_BACK || te == TOUCH_BACK) {
      done = true;
    }
    delay(8);
  }

  scanner->clearResults();
  uiToast("BLE scan encerrado");
}
