#pragma once
#include <Arduino.h>
void packetMonitorStart(uint8_t channel=0);
void packetMonitorStop();
uint32_t packetCount();
