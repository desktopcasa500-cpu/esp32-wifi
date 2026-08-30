#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
extern TFT_eSPI tft;
void uiBegin();
void uiHeader(const String& title);
void uiFooter();
void uiMessage(const String& text);
void uiMenu(const String items[], size_t count, int selected);
