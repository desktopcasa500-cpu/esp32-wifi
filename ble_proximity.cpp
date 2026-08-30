#include "ble_proximity.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include "ui.h"
#include "buttons.h"
#include "touch.h"
#include <math.h>

static float estimateDistance(int rssi) {
  if (rssi >= 0) return 0.0f;
  // Heuristica para BLE; ambiente, antena e potencia real alteram o resultado.
  const float txPower = -59.0f;
  const float pathLoss = 2.2f;
  return powf(10.0f, (txPower - rssi) / (10.0f * pathLoss));
}

static void drawGraph(const int16_t* history, int count, int threshold) {
  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / PROXIMITY", "LIVE");
  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString("RSSI dBm", 8, 37);

  const int left = 25;
  const int right = 312;
  const int top = 55;
  const int bottom = 170;
  tft.drawRect(left, top, right - left, bottom - top, 0x39C7);

  int yThreshold = map(constrain(threshold, -100, -30), -100, -30, bottom - 4, top + 4);
  for (int x = left + 1; x < right - 1; x += 4) tft.drawFastHLine(x, yThreshold, 3, 0xF800);

  if (count > 1) {
    for (int i = 1; i < count; ++i) {
      const int x1 = map(i - 1, 0, max(1, count - 1), left + 2, right - 2);
      const int x2 = map(i, 0, max(1, count - 1), left + 2, right - 2);
      const int y1 = map(constrain((int)history[i - 1], -100, -30), -100, -30, bottom - 4, top + 4);
      const int y2 = map(constrain((int)history[i], -100, -30), -100, -30, bottom - 4, top + 4);
      tft.drawLine(x1, y1, x2, y2, TFT_CYAN);
    }
  }

  if (count > 0) {
    const int rssi = history[count - 1];
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(rssi) + " dBm", 8, 181);
    tft.drawRightString(String(estimateDistance(rssi), 1) + " m", 310, 181);
  }
  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString("threshold " + String(threshold) + " dBm", 8, 198);
  uiFooter();
}

void bleProximityMonitor(const String& address, uint32_t durationMs) {
  if (address.length() < 10) {
    uiMessage("Endereco BLE invalido.", "BLE / PROXIMITY");
    return;
  }

  BLEDevice::init("");
  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);

  int16_t history[100] = {};
  int historyCount = 0;
  int minRssi = 127;
  int maxRssi = -127;
  uint32_t samples = 0;
  const int threshold = -65;
  const uint32_t endAt = millis() + durationMs;

  while (millis() < endAt) {
    BLEScanResults* results = scan->start(1, false);
    if (results) {
      for (int i = 0; i < results->getCount(); ++i) {
        BLEAdvertisedDevice d = results->getDevice(i);
        const String seen = d.getAddress().toString().c_str();
        if (!seen.equalsIgnoreCase(address)) continue;

        const int rssi = d.getRSSI();
        if (historyCount < 100) history[historyCount++] = rssi;
        else {
          memmove(history, history + 1, sizeof(history) - sizeof(history[0]));
          history[99] = rssi;
        }
        minRssi = min(minRssi, rssi);
        maxRssi = max(maxRssi, rssi);
        ++samples;
        drawGraph(history, historyCount, threshold);
        if (rssi >= threshold) uiToast("Inside threshold", 350);
      }
      scan->clearResults();
    }

    const ButtonEvent be = buttonsRead();
    const TouchEvent te = touchRead();
    if (be == BE_BACK || te == TOUCH_BACK) break;
    delay(20);
  }

  scan->clearResults();
  uiMessage(
    "Target: " + address +
    "\nSamples: " + String(samples) +
    "\nMin RSSI: " + String(samples ? minRssi : 0) + " dBm" +
    "\nMax RSSI: " + String(samples ? maxRssi : 0) + " dBm" +
    "\nThreshold: " + String(threshold) + " dBm",
    "BLE / RESULT"
  );
}
