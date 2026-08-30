#pragma once
#include <Arduino.h>
struct ChannelScore { uint8_t channel; int score; uint8_t networks; };
ChannelScore optimizeChannels();
void showChannelOptimizer();
