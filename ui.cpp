#include "ui.h"
#include "config.h"
#include "settings.h"
#include "touch.h"
#include <WiFi.h>

TFT_eSPI tft = TFT_eSPI();

namespace {
const uint16_t BG = TFT_BLACK;
const uint16_t BAR = 0x18E3;
const uint16_t PANEL = 0x10A2;
const uint16_t TEXT = 0xD6BA;
const uint16_t MUTED = 0x7BEF;
const uint16_t ACCENT = TFT_CYAN;
const uint16_t GOOD = 0x05E0;
const uint16_t SELECTED = 0x2945;

void drawWrapped(const String& text, int x, int y, int maxWidth, int lineHeight) {
  String line;
  for (size_t i = 0; i <= text.length(); ++i) {
    const char c = (i < text.length()) ? text[i] : '\n';
    if (c == '\n') {
      if (y <= UI_FOOTER_Y - 8) tft.drawString(line, x, y, UI_TEXT_FONT);
      y += lineHeight;
      line = "";
      continue;
    }

    String candidate = line + c;
    if (tft.textWidth(candidate, 1) > maxWidth && line.length()) {
      if (y <= UI_FOOTER_Y - 8) tft.drawString(line, x, y, 1);
      y += lineHeight;
      line = String(c);
    } else {
      line = candidate;
    }
  }
}

ButtonEvent readAnyInput() {
  const ButtonEvent button = buttonsRead();
  if (button != BE_NONE) return button;

  switch (touchRead()) {
    case TOUCH_PREV: return BE_PREV;
    case TOUCH_SELECT: return BE_SELECT;
    case TOUCH_NEXT: return BE_NEXT;
    case TOUCH_BACK: return BE_BACK;
    default: return BE_NONE;
  }
}
}

void uiBegin() {
  tft.init();
  tft.setRotation(1);
  tft.setTextFont(UI_TEXT_FONT);
  tft.setTextSize(1);
  tft.setTextWrap(false, false);
  tft.fillScreen(BG);
  uiSetBrightness(settings.brightness);
}

void uiSetBrightness(uint8_t value) {
  pinMode(TFT_LED, OUTPUT);
  analogWrite(TFT_LED, value);
}

void uiHeader(const String& title, const String& status) {
  tft.fillRect(0, 0, UI_SCREEN_WIDTH, UI_HEADER_H, BAR);
  tft.drawFastHLine(0, UI_HEADER_H - 1, UI_SCREEN_WIDTH, ACCENT);
  tft.setTextColor(TEXT, BAR);
  tft.drawString(title.substring(0, 28), 8, 8, 1);
  if (status.length()) {
    tft.setTextColor(GOOD, BAR);
    tft.drawRightString(status.substring(0, 9), 312, 8, 1);
  }
}

void uiFooter() {
  tft.fillRect(0, UI_FOOTER_Y, UI_SCREEN_WIDTH, UI_FOOTER_H, BAR);
  tft.drawFastHLine(0, UI_FOOTER_Y, UI_SCREEN_WIDTH, 0x4A69);
  tft.setTextColor(MUTED, BAR);
  tft.drawString("PREV", 8, 219, 1);
  tft.drawCentreString("SELECT", 160, 219, 1);
  tft.drawRightString("NEXT", 312, 219, 1);
}

void uiMessage(const String& text, const String& title) {
  tft.fillScreen(BG);
  uiHeader(title);
  tft.setTextColor(TEXT, BG);
  drawWrapped(text, 8, 43, 304, 17);
  uiFooter();

  delay(80);
  while (readAnyInput() == BE_NONE) delay(8);
}

void uiMenu(const char* const items[], size_t count, int selected) {
  if (count == 0) return;
  if (selected < 0) selected = 0;
  if (selected >= static_cast<int>(count)) selected = static_cast<int>(count) - 1;

  tft.fillScreen(BG);
  uiHeader("ESP32 / TOOLKIT", WiFi.status() == WL_CONNECTED ? "LINK" : "LOCAL");

  const int pageSize = 7;
  const int first = (selected / pageSize) * pageSize;
  const int last = min(static_cast<int>(count), first + pageSize);

  for (int i = first; i < last; ++i) {
    const int row = i - first;
    const int y = 39 + row * 24;
    const bool active = i == selected;
    const uint16_t bg = active ? SELECTED : BG;
    if (active) {
      tft.fillRoundRect(5, y - 3, 310, 21, 3, bg);
      tft.drawFastVLine(5, y - 3, 21, ACCENT);
    }
    tft.setTextColor(active ? TFT_WHITE : TEXT, bg);
    tft.drawString(String(i + 1) + "  " + String(items[i]).substring(0, 27), 12, y, 1);
  }

  tft.setTextColor(MUTED, BG);
  tft.drawString(String(selected + 1) + "/" + String(count), 8, 194, 1);
  uiFooter();
}

void uiProgress(const String& title, const String& detail, uint8_t percent) {
  percent = min<uint8_t>(percent, 100);
  tft.fillScreen(BG);
  uiHeader(title);
  tft.setTextColor(TEXT, BG);
  tft.drawString(detail.substring(0, 42), 8, 50, 1);
  tft.drawRoundRect(8, 91, 304, 18, 3, PANEL);
  const int width = map(percent, 0, 100, 0, 300);
  if (width > 0) tft.fillRect(10, 94, width, 12, ACCENT);
  tft.setTextColor(MUTED, BG);
  tft.drawRightString(String(percent) + "%", 310, 122, 1);
  uiFooter();
}

void uiToast(const String& text, uint16_t ms) {
  const String shown = text.substring(0, 34);
  const int width = constrain(tft.textWidth(shown, 1) + 28, 120, 306);
  const int x = (UI_SCREEN_WIDTH - width) / 2;
  tft.fillRoundRect(x, 93, width, 31, 4, PANEL);
  tft.drawRoundRect(x, 93, width, 31, 4, ACCENT);
  tft.setTextColor(TFT_WHITE, PANEL);
  tft.drawCentreString(shown, 160, 102, 1);
  delay(ms);
}

ButtonEvent uiReadInput() {
  return readAnyInput();
}
