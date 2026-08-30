#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

void uiBegin();
void uiSetBrightness(uint8_t value);
void uiHeader(const String& title, const String& status = "");
void uiFooter();
void uiMessage(const String& text, const String& title = "ESP32 / TOOLKIT");
void uiMenu(const String items[], size_t count, int selected);
void uiProgress(const String& title, const String& detail, uint8_t percent);
void uiToast(const String& text, uint16_t ms = 900);
