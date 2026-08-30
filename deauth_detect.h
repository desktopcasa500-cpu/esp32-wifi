#pragma once
#include <Arduino.h>

void deauthDetectorStart(uint8_t channel = 0);
void deauthDetectorStop();
uint32_t deauthEvents();
uint32_t disassocEvents();
int deauthLastRssi();
uint8_t deauthChannel();
