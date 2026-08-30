#pragma once
#include <Arduino.h>
struct PasswordReport { uint8_t score; float entropy; bool common; bool hasSequence; };
PasswordReport analyzePassword(const String& password);
String passwordAdvice(const PasswordReport& r);
