#pragma once
#include <Arduino.h>

struct PacketStats {
  uint32_t total;
  uint32_t beacon;
  uint32_t probe;
  uint32_t data;
  uint32_t mgmt;
  uint32_t control;
  uint16_t pps;
};

void packetMonitorStart(uint8_t channel = 0);
void packetMonitorStop();
uint32_t packetCount();
PacketStats packetMonitorStats();
