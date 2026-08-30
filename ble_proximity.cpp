#include "ble_proximity.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include "ui.h"
#include <math.h>
#include <string.h>

namespace {
const int PROXIMITY_THRESHOLD = -65;

float estimateDistance(int rssi) {
  if (rssi >= 0) return 0.0f;
  const float txPower = -59.0f;
  const float pathLoss = 2.2f;
  return powf(10.0f, (txPower - rssi) / (10.0f * pathLoss));
}

void drawGraph(const int16_t* history, uint8_t count, int lastRssi) {
  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / PROXIMITY", "LIVE");
  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString("RSSI history", 8, 38, 1);

  const int left = 15, top = 57, width = 290, height = 105;
  tft.drawRect(left, top, width, height, 0x39C7);

  const int thresholdY = map(constrain(PROXIMITY_THRESHOLD, -100, -30), -100, -30,
                             top + height - 3, top + 3);
  for (int x = left + 2; x < left + width - 2; x += 5) {
    tft.drawFastHLine(x, thresholdY, 3, TFT_RED);
  }

  if (count > 1) {
    for (uint8_t i = 1; i < count; ++i) {
      const int x1 = map(i - 1, 0, max(1, (int)count - 1), left + 2, left + width - 3);
      const int x2 = map(i, 0, max(1, (int)count - 1), left + 2, left + width - 3);
      const int y1 = map(constrain((int)history[i - 1], -100, -30), -100, -30,
                         top + height - 3, top + 3);
      const int y2 = map(constrain((int)history[i], -100, -30), -100, -30,
                         top + height - 3, top + 3);
      tft.drawLine(x1, y1, x2, y2, TFT_CYAN);
    }
  }

  if (lastRssi > -127) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(lastRssi) + " dBm", 12, 172, 2);
    tft.drawRightString(String(estimateDistance(lastRssi), 1) + " m", 308, 172, 2);
  }
  tft.setTextColor(0x7BEF, TFT_BLACK);
  tft.drawString("Threshold " + String(PROXIMITY_THRESHOLD) + " dBm", 12, 193, 1);
  uiFooter();
}
}

void bleProximityMonitor(const String& address, uint32_t durationMs) {
  if (address.length() != 17) {
    uiMessage("Endereco BLE invalido.", "BLE / PROXIMITY");
    return;
  }

  BLEDevice::init("");
  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);

  int16_t history[100] = {0};
  uint8_t historyCount = 0;
  int minRssi = 127;
  int maxRssi = -127;
  int lastRssi = -127;
  uint32_t samples = 0;
  const uint32_t started = millis();

  while ((uint32_t)(millis() - started) < durationMs) {
    if (uiReadInput() == BE_BACK) break;

    BLEScanResults results = scan->start(1, false);
    for (int i = 0; i < results.getCount(); ++i) {
      BLEAdvertisedDevice device = results.getDevice(i);
      const String seen = device.getAddress().toString().c_str();
      if (!seen.equalsIgnoreCase(address)) continue;

      lastRssi = device.getRSSI();
      minRssi = min(minRssi, lastRssi);
      maxRssi = max(maxRssi, lastRssi);
      ++samples;

      if (historyCount < 100) {
        history[historyCount++] = static_cast<int16_t>(lastRssi);
      } else {
        memmove(history, history + 1, sizeof(history) - sizeof(history[0]));
        history[99] = static_cast<int16_t>(lastRssi);
      }
      drawGraph(history, historyCount, lastRssi);
    }
    scan->clearResults();
    delay(20);
  }

  uiMessage("Target: " + address +
            "\nSamples: " + String(samples) +
            "\nMin: " + String(samples ? minRssi : 0) + " dBm" +
            "\nMax: " + String(samples ? maxRssi : 0) + " dBm" +
            "\nThreshold: " + String(PROXIMITY_THRESHOLD) + " dBm",
            "BLE / RESULT");
}
