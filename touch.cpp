#include "touch.h"
#include "config.h"
#include "ui.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

namespace {
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
SPIClass touchSPI(HSPI);
TouchCalibration calibration = {
  DEFAULT_TOUCH_MIN_X, DEFAULT_TOUCH_MAX_X,
  DEFAULT_TOUCH_MIN_Y, DEFAULT_TOUCH_MAX_Y
};
bool pressed = false;
uint32_t lastEventAt = 0;

bool validCalibration(const TouchCalibration& c) {
  return c.minX >= 0 && c.maxX <= 4095 &&
         c.minY >= 0 && c.maxY <= 4095 &&
         c.minX + 200 < c.maxX && c.minY + 200 < c.maxY;
}

int screenX(int rawX) {
  return constrain(map(rawX, calibration.minX, calibration.maxX, 0, 319), 0, 319);
}

int screenY(int rawY) {
  return constrain(map(rawY, calibration.minY, calibration.maxY, 0, 239), 0, 239);
}
}

void touchBegin() {
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  touch.begin(touchSPI);
  touch.setRotation(1);
  pressed = false;
  lastEventAt = 0;
}

void touchSetCalibration(const TouchCalibration& c) {
  calibration = validCalibration(c) ? c : TouchCalibration{
    DEFAULT_TOUCH_MIN_X, DEFAULT_TOUCH_MAX_X,
    DEFAULT_TOUCH_MIN_Y, DEFAULT_TOUCH_MAX_Y
  };
  pressed = false;
}

TouchCalibration touchGetCalibration() {
  return calibration;
}

TouchEvent touchRead() {
  const uint32_t now = millis();
  if (!touch.touched()) {
    pressed = false;
    return TOUCH_NONE;
  }

  if (pressed || (uint32_t)(now - lastEventAt) < 90U) {
    return TOUCH_NONE;
  }

  TS_Point p = touch.getPoint();
  const int x = screenX(p.x);
  const int y = screenY(p.y);

  pressed = true;
  lastEventAt = now;

  if (y >= UI_FOOTER_Y) {
    if (x < 106) return TOUCH_PREV;
    if (x < 213) return TOUCH_SELECT;
    return TOUCH_NEXT;
  }

  // Back zone is the upper-left corner.
  if (x < 60 && y < UI_HEADER_H) return TOUCH_BACK;
  return TOUCH_NONE;
}

bool touchCalibrate(TouchCalibration& out) {
  const int16_t targetX[4] = { 18, 301, 301, 18 };
  const int16_t targetY[4] = { 44, 44, 190, 190 };
  int16_t rawX[4] = { 0, 0, 0, 0 };
  int16_t rawY[4] = { 0, 0, 0, 0 };

  tft.fillScreen(TFT_BLACK);
  uiHeader("TOUCH / CALIBRATION");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Toque cada ponto.", 8, 52, 2);

  for (uint8_t i = 0; i < 4; ++i) {
    tft.fillCircle(targetX[i], targetY[i], 5, TFT_CYAN);

    const uint32_t waitStart = millis();
    while (!touch.touched()) {
      if ((uint32_t)(millis() - waitStart) > 20000U) {
        tft.fillCircle(targetX[i], targetY[i], 5, TFT_BLACK);
        uiMessage("Tempo esgotado.", "TOUCH / ERROR");
        return false;
      }
      delay(5);
    }

    delay(25);
    TS_Point p = touch.getPoint();
    rawX[i] = p.x;
    rawY[i] = p.y;

    while (touch.touched()) delay(5);
    tft.fillCircle(targetX[i], targetY[i], 5, TFT_BLACK);
  }

  TouchCalibration result;
  result.minX = min(min(rawX[0], rawX[1]), min(rawX[2], rawX[3]));
  result.maxX = max(max(rawX[0], rawX[1]), max(rawX[2], rawX[3]));
  result.minY = min(min(rawY[0], rawY[1]), min(rawY[2], rawY[3]));
  result.maxY = max(max(rawY[0], rawY[1]), max(rawY[2], rawY[3]));

  if (!validCalibration(result)) {
    uiMessage("Calibracao invalida.\n\nTente novamente mantendo o toque dentro dos pontos.", "TOUCH / ERROR");
    return false;
  }

  touchSetCalibration(result);
  out = calibration;
  uiMessage("Calibracao concluida.", "TOUCH / OK");
  return true;
}
