#pragma once
#include <Arduino.h>
enum TouchEvent { TOUCH_NONE, TOUCH_PREV, TOUCH_SELECT, TOUCH_NEXT, TOUCH_BACK };
void touchBegin();
TouchEvent touchRead();
