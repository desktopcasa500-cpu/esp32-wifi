#include "buttons.h"
#include "config.h"

namespace {
const uint8_t kPins[4] = { BTN_PREV, BTN_NEXT, BTN_SELECT, BTN_BACK };
bool stableState[4] = { HIGH, HIGH, HIGH, HIGH };
bool lastRawState[4] = { HIGH, HIGH, HIGH, HIGH };
uint32_t changedAt[4] = { 0, 0, 0, 0 };
}

void buttonsBegin() {
  const uint32_t now = millis();
  for (uint8_t i = 0; i < 4; ++i) {
    pinMode(kPins[i], INPUT_PULLUP);
    const bool state = digitalRead(kPins[i]);
    stableState[i] = state;
    lastRawState[i] = state;
    changedAt[i] = now;
  }
}

ButtonEvent buttonsRead() {
  const uint32_t now = millis();
  for (uint8_t i = 0; i < 4; ++i) {
    const bool raw = digitalRead(kPins[i]);

    if (raw != lastRawState[i]) {
      lastRawState[i] = raw;
      changedAt[i] = now;
    }

    if (raw != stableState[i] && (uint32_t)(now - changedAt[i]) >= 30U) {
      stableState[i] = raw;
      if (stableState[i] == LOW) {
        return static_cast<ButtonEvent>(i + 1);
      }
    }
  }
  return BE_NONE;
}
