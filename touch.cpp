#include "touch.h"
#include "config.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

static XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
static SPIClass touchSPI(HSPI);
static bool pressed = false;
static uint32_t lastEvent = 0;

// Valores iniciais para um XPT2046 comum em 240x320. Ajuste em Settings > Touch.
static int16_t minX = 200;
static int16_t maxX = 3900;
static int16_t minY = 200;
static int16_t maxY = 3900;

void touchBegin() {
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  touch.begin(touchSPI);
  touch.setRotation(1);
}

TouchEvent touchRead() {
  if (!touch.touched()) {
    pressed = false;
    return TOUCH_NONE;
  }

  if (pressed || millis() - lastEvent < 90) return TOUCH_NONE;
  pressed = true;
  lastEvent = millis();

  TS_Point p = touch.getPoint();
  const int x = constrain(map(p.x, minX, maxX, 0, 319), 0, 319);
  const int y = constrain(map(p.y, minY, maxY, 0, 239), 0, 239);

  if (y >= 205) {
    if (x < 106) return TOUCH_PREV;
    if (x < 213) return TOUCH_SELECT;
    return TOUCH_NEXT;
  }

  if (x < 55 && y < 35) return TOUCH_BACK;
  return TOUCH_NONE;
}
