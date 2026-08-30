#pragma once
#include <Arduino.h>
enum ButtonEvent { BE_NONE, BE_PREV, BE_NEXT, BE_SELECT, BE_BACK };
void buttonsBegin();
ButtonEvent buttonsRead();
