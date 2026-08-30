#include "touch.h"
#include "config.h"
#include "ui.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

static XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
static SPIClass touchSPI(HSPI);
static bool pressed = false;
static uint32_t lastEvent = 0;
static TouchCalibration calibration = {200, 3900, 200, 3900};

static int readAverageX() {
  int32_t total = 0;
  for (uint8_t i = 0; i < 4; ++i) { total += touch.getPoint().x; delay(3); }
  return total / 4;
}

static int readAverageY() {
  int32_t total = 0;
  for (uint8_t i = 0; i < 4; ++i) { total += touch.getPoint().y; delay(3); }
  return total / 4;
}

void touchBegin() {
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  touch.begin(touchSPI);
  touch.setRotation(1);
}

void touchSetCalibration(const TouchCalibration& c) {
  calibration = c;
  if (calibration.minX >= calibration.maxX) { calibration.minX = 200; calibration.maxX = 3900; }
  if (calibration.minY >= calibration.maxY) { calibration.minY = 200; calibration.maxY = 3900; }
}

TouchCalibration touchGetCalibration() { return calibration; }

TouchEvent touchRead() {
  if (!touch.touched()) {
    pressed = false;
    return TOUCH_NONE;
  }

  if (pressed || millis() - lastEvent < 90) return TOUCH_NONE;
  pressed = true;
  lastEvent = millis();

  const TS_Point p = touch.getPoint();
  const int x = constrain(map(p.x, calibration.minX, calibration.maxX, 0, 319), 0, 319);
  const int y = constrain(map(p.y, calibration.minY, calibration.maxY, 0, 239), 0, 239);

  if (y >= 205) {
    if (x < 106) return TOUCH_PREV;
    if (x < 213) return TOUCH_SELECT;
    return TOUCH_NEXT;
  }
  if (x < 55 && y < 35) return TOUCH_BACK;
  return TOUCH_NONE;
}

bool touchCalibrate(TouchCalibration& out) {
  const int16_t tx[4] = {25, 294, 294, 25};
  const int16_t ty[4] = {38, 38, 185, 185};
  int16_t rawX[4] = {}, rawY[4] = {};

  tft.fillScreen(TFT_BLACK);
  uiHeader("TOUCH / CALIBRATION");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Toque nos 4 pontos.", 8, 48);

  for (uint8_t i = 0; i < 4; ++i) {
    tft.fillCircle(tx[i], ty[i], 4, TFT_CYAN);
    while (!touch.touched()) delay(10);
    delay(30);
    TS_Point p = touch.getPoint();
    rawX[i] = p.x;
    rawY[i] = p.y;
    while (touch.touched()) delay(10);
    tft.fillCircle(tx[i], ty[i], 4, TFT_BLACK);
  }

  const int16_t minX = min(rawX[0], min(rawX[1], min(rawX[2], rawX[3])));
  const int16_t maxX = max(rawX[0], max(rawX[1], max(rawX[2], rawX[3])));
  const int16_t minY = min(rawY[0], min(rawY[1], min(rawY[2], rawY[3])));
  const int16_t maxY = max(rawY[0], max(rawY[1], max(rawY[2], rawY[3])));

  if (maxX - minX < 500 || maxY - minY < 500) {
    uiMessage("Calibracao rejeitada.\n\nA variacao dos pontos foi insuficiente.", "TOUCH / ERROR");
    return false;
  }

  out.minX = minX;
  out.maxX = maxX;
  out.minY = minY;
  out.maxY = maxY;
  touchSetCalibration(out);
  uiMessage("Calibracao concluida.\n\nOs novos valores foram aplicados.", "TOUCH / OK");
  return true;
}
