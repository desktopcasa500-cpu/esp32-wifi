#include "password_strength.h"
#include <Arduino.h>
#include <math.h>

namespace {
bool hasSequence(const String& value) {
  if (value.length() < 3) return false;
  for (size_t i = 0; i + 2 < value.length(); ++i) {
    const int a = static_cast<uint8_t>(value[i]);
    const int b = static_cast<uint8_t>(value[i + 1]);
    const int c = static_cast<uint8_t>(value[i + 2]);
    if ((b == a + 1 && c == b + 1) || (b == a - 1 && c == b - 1)) return true;
  }
  return false;
}

bool hasRepeatedRun(const String& value) {
  for (size_t i = 2; i < value.length(); ++i) {
    if (value[i] == value[i - 1] && value[i] == value[i - 2]) return true;
  }
  return false;
}

bool isCommon(const String& value) {
  static const char* const COMMON[] = {
    "password", "password1", "123456", "12345678", "123456789",
    "qwerty", "qwerty123", "admin", "admin123", "letmein", "welcome",
    "abc123", "iloveyou", "000000", "00000000", "111111", "123123",
    "senha", "senha123", "1q2w3e4r", "qwertyuiop"
  };
  for (size_t i = 0; i < sizeof(COMMON) / sizeof(COMMON[0]); ++i) {
    if (value.equalsIgnoreCase(COMMON[i])) return true;
  }
  return value.length() < 8;
}
}

PasswordReport analyzePassword(const String& password) {
  bool lower = false;
  bool upper = false;
  bool digit = false;
  bool symbol = false;

  for (size_t i = 0; i < password.length(); ++i) {
    const char c = password[i];
    if (c >= 'a' && c <= 'z') lower = true;
    else if (c >= 'A' && c <= 'Z') upper = true;
    else if (c >= '0' && c <= '9') digit = true;
    else symbol = true;
  }

  int pool = 0;
  if (lower) pool += 26;
  if (upper) pool += 26;
  if (digit) pool += 10;
  if (symbol) pool += 33;

  const float entropy = pool > 0
    ? static_cast<float>(password.length() * log2(static_cast<double>(pool)))
    : 0.0f;
  const bool common = isCommon(password);
  const bool sequence = hasSequence(password);
  const bool repeated = hasRepeatedRun(password);

  int score = 0;
  if (password.length() >= 8) score += 25;
  if (password.length() >= 12) score += 20;
  if (password.length() >= 16) score += 15;
  if (lower && upper) score += 10;
  if (digit) score += 10;
  if (symbol) score += 10;
  if (common) score -= 35;
  if (sequence) score -= 15;
  if (repeated) score -= 10;

  PasswordReport result;
  result.score = static_cast<uint8_t>(constrain(score, 0, 100));
  result.entropy = entropy;
  result.common = common;
  result.hasSequence = sequence;
  return result;
}

String passwordAdvice(const PasswordReport& report) {
  if (report.common) return "Use uma senha longa e exclusiva; evite listas comuns.";
  if (report.hasSequence) return "Evite sequencias previsiveis como 123 ou abc.";
  if (report.score < 40) return "Aumente o comprimento e use mais diversidade.";
  if (report.score < 70) return "Boa base. Mais comprimento ajuda bastante.";
  return "Boa composicao. Mantenha a senha exclusiva.";
}
