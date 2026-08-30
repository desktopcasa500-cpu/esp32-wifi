#pragma once
#include <Arduino.h>
void bleAdvertiserStart(uint8_t profile=0,uint16_t intervalMs=500);
void bleAdvertiserStop();
