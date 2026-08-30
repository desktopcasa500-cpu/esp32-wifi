#pragma once
#include <Arduino.h>

void bleScan(uint32_t seconds = 5);
String bleLastSelectedAddress();
String bleLastSelectedName();
bool bleHasSelection();
