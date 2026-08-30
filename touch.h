#pragma once
#include <Arduino.h>

enum TouchEvent { TOUCH_NONE, TOUCH_PREV, TOUCH_SELECT, TOUCH_NEXT, TOUCH_BACK };

struct TouchCalibration {
  int16_t minX;
  int16_t maxX;
  int16_t minY;
  int16_t maxY;
};

void touchBegin();
TouchEvent touchRead();
void touchSetCalibration(const TouchCalibration& calibration);
TouchCalibration touchGetCalibration();
bool touchCalibrate(TouchCalibration& calibration);
