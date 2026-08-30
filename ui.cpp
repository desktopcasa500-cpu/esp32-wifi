#include "ui.h"

TFT_eSPI tft = TFT_eSPI();

static const uint16_t BG = TFT_BLACK;
static const uint16_t BAR = 0x18E3;
static const uint16_t TEXT = 0xD6BA;
static const uint16_t MUTED = 0x7BEF;
static const uint16_t ACCENT = TFT_CYAN;
static const uint16_t SELECTED = 0x39C7;

void uiBegin() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BG);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextWrap(false);
}

void uiHeader(const String& title) {
  tft.fillRect(0, 0, 320, 30, BAR);
  tft.drawFastHLine(0, 29, 320, ACCENT);
  tft.setTextColor(TEXT, BAR);
  tft.setCursor(8, 8);
  tft.print(title.substring(0, 38));
}

void uiFooter() {
  tft.fillRect(0, 211, 320, 29, BAR);
  tft.drawFastHLine(0, 211, 320, 1, 0x4A69);
  tft.setTextColor(MUTED, BAR);
  tft.drawString("UP", 18, 219);
  tft.drawString("SELECT", 136, 219);
  tft.drawString("DOWN", 265, 219);
}

void uiMessage(const String& text) {
  tft.fillScreen(BG);
  uiHeader("ESP32 / TOOLKIT");
  tft.setTextColor(TEXT, BG);

  String line;
  int y = 43;
  for (size_t i = 0; i <= text.length(); ++i) {
    const char c = (i < text.length()) ? text[i] : '\n';
    if (c == '\n') {
      if (y <= 196) tft.drawString(line.substring(0, 40), 8, y);
      y += 20;
      line = "";
    } else {
      line += c;
      if (line.length() >= 40) {
        if (y <= 196) tft.drawString(line, 8, y);
        y += 20;
        line = "";
      }
    }
  }
  uiFooter();
}

void uiMenu(const String items[], size_t count, int selected) {
  tft.fillScreen(BG);
  uiHeader("ESP32 / TOOLKIT");

  const int first = max(0, min(selected - 3, (int)count - 7));
  const int last = min((int)count, first + 7);

  for (int i = first; i < last; ++i) {
    const int row = i - first;
    const int y = 39 + row * 24;
    const bool active = (i == selected);

    if (active) {
      tft.fillRect(5, y - 3, 310, 21, SELECTED);
      tft.drawFastVLine(5, y - 3, 21, ACCENT);
    }

    tft.setTextColor(active ? TFT_WHITE : TEXT, active ? SELECTED : BG);
    tft.drawString(String(i + 1) + "  " + items[i].substring(0, 28), 12, y);
  }

  tft.setTextColor(MUTED, BG);
  tft.drawRightString(String(selected + 1) + "/" + String(count), 310, 195);
  uiFooter();
}
