#include "ui.h"
#include "settings.h"

TFT_eSPI tft = TFT_eSPI();

static const uint16_t BG = 0x0000;
static const uint16_t BAR = 0x18E3;
static const uint16_t PANEL = 0x10A2;
static const uint16_t TEXT = 0xD6BA;
static const uint16_t MUTED = 0x7BEF;
static const uint16_t ACCENT = TFT_CYAN;
static const uint16_t GOOD = 0x05E0;
static const uint16_t SELECTED = 0x2945;

static void wrapText(const String& text, int x, int y, int maxChars, int lineHeight) {
  String line;
  for (size_t i = 0; i <= text.length(); ++i) {
    const char c = (i < text.length()) ? text[i] : '\n';
    if (c == '\n') {
      if (y < 202) tft.drawString(line.substring(0, maxChars), x, y);
      y += lineHeight;
      line = "";
    } else {
      line += c;
      if ((int)line.length() >= maxChars) {
        if (y < 202) tft.drawString(line, x, y);
        y += lineHeight;
        line = "";
      }
    }
  }
}

void uiBegin() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BG);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextWrap(false);
  uiSetBrightness(settings.brightness);
}

void uiSetBrightness(uint8_t value) {
  pinMode(TFT_LED, OUTPUT);
  analogWrite(TFT_LED, value);
}

void uiHeader(const String& title, const String& status) {
  tft.fillRect(0, 0, 320, 31, BAR);
  tft.drawFastHLine(0, 30, 320, ACCENT);
  tft.setTextColor(TEXT, BAR);
  tft.drawString(title.substring(0, 29), 8, 8);

  if (status.length()) {
    tft.setTextColor(GOOD, BAR);
    tft.drawRightString(status.substring(0, 8), 312, 8);
  }
}

void uiFooter() {
  tft.fillRect(0, 211, 320, 29, BAR);
  tft.drawFastHLine(0, 211, 320, 1, 0x4A69);
  tft.setTextColor(MUTED, BAR);
  tft.drawString("UP", 18, 219);
  tft.drawCentreString("SELECT", 160, 219, 2);
  tft.drawRightString("DOWN", 302, 219);
}

void uiMessage(const String& text, const String& title) {
  tft.fillScreen(BG);
  uiHeader(title);
  tft.setTextColor(TEXT, BG);
  wrapText(text, 8, 42, 39, 19);
  uiFooter();
}

void uiMenu(const String items[], size_t count, int selected) {
  tft.fillScreen(BG);
  uiHeader("ESP32 / TOOLKIT", WiFi.status() == WL_CONNECTED ? "LINK" : "LOCAL");

  const int pageSize = 7;
  const int page = selected / pageSize;
  const int first = page * pageSize;
  const int last = min((int)count, first + pageSize);

  for (int i = first; i < last; ++i) {
    const int row = i - first;
    const int y = 41 + row * 23;
    const bool active = i == selected;

    if (active) {
      tft.fillRoundRect(5, y - 3, 310, 20, 2, SELECTED);
      tft.drawFastVLine(5, y - 3, 20, ACCENT);
    }

    tft.setTextColor(active ? TFT_WHITE : TEXT, active ? SELECTED : BG);
    tft.drawString(String(i + 1) + "  " + items[i].substring(0, 28), 12, y);
  }

  tft.setTextColor(MUTED, BG);
  tft.drawRightString(String(selected + 1) + "/" + String(count), 310, 196);
  uiFooter();
}

void uiProgress(const String& title, const String& detail, uint8_t percent) {
  tft.fillScreen(BG);
  uiHeader(title);
  tft.setTextColor(TEXT, BG);
  tft.drawString(detail.substring(0, 39), 8, 55);
  tft.drawRect(8, 94, 304, 16, PANEL);
  const int w = map(percent, 0, 100, 0, 300);
  if (w > 0) tft.fillRect(10, 96, w, 12, ACCENT);
  tft.setTextColor(MUTED, BG);
  tft.drawRightString(String(percent) + "%", 310, 125);
  uiFooter();
}

void uiToast(const String& text, uint16_t ms) {
  const int width = min(306, max(120, (int)text.length() * 7));
  const int x = (320 - width) / 2;
  tft.fillRoundRect(x, 92, width, 34, 4, PANEL);
  tft.drawRoundRect(x, 92, width, 34, 4, ACCENT);
  tft.setTextColor(TFT_WHITE, PANEL);
  tft.drawCentreString(text.substring(0, 40), 160, 103, 2);
  delay(ms);
}
