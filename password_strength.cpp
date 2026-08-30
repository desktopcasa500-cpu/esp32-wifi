#include "password_strength.h"
#include <math.h>

static bool hasSequence(const String& s) {
  if (s.length() < 3) return false;
  for (size_t i = 0; i + 2 < s.length(); ++i) {
    const int a = (uint8_t)s[i];
    const int b = (uint8_t)s[i + 1];
    const int c = (uint8_t)s[i + 2];
    if ((b == a + 1 && c == b + 1) || (b == a - 1 && c == b - 1)) return true;
  }
  return false;
}

static bool hasRepeatedRun(const String& s) {
  for (size_t i = 2; i < s.length(); ++i) {
    if (s[i] == s[i - 1] && s[i] == s[i - 2]) return true;
  }
  return false;
}

static bool isCommon(const String& p) {
  static const char* common[] = {
    "password", "password1", "123456", "12345678", "123456789",
    "qwerty", "qwerty123", "admin", "letmein", "welcome", "abc123",
    "iloveyou", "000000", "111111", "senha", "senha123",
    "123123", "admin123", "00000000", "1q2w3e4r", "qwertyuiop"
  };
  if (p.length() < 8) return true;
  for (const char* word : common) {
    if (p.equalsIgnoreCase(word)) return true;
  }
  return false;
}

PasswordReport analyzePassword(const String& p) {
  bool lower = false, upper = false, digit = false, symbol = false;
  for (size_t i = 0; i < p.length(); ++i) {
    const char c = p[i];
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

  const float entropy = pool ? p.length() * log2((double)pool) : 0.0f;
  const bool common = isCommon(p);
  const bool sequence = hasSequence(p);
  const bool repeated = hasRepeatedRun(p);

  int score = 0;
  if (p.length() >= 8) score += 25;
  if (p.length() >= 12) score += 20;
  if (p.length() >= 16) score += 15;
  if (lower && upper) score += 10;
  if (digit) score += 10;
  if (symbol) score += 10;
  if (common) score -= 35;
  if (sequence) score -= 15;
  if (repeated) score -= 10;

  score = constrain(score, 0, 100);
  return {(uint8_t)score, entropy, common, sequence};
}

String passwordAdvice(const PasswordReport& r) {
  if (r.common) return "Muito previsivel. Use uma frase longa e exclusiva.";
  if (r.hasSequence) return "Evite sequencias como abc, 123 ou equivalentes.";
  if (r.score < 40) return "Aumente bastante o comprimento e varie os caracteres.";
  if (r.score < 70) return "Boa base. Mais comprimento melhora a resistencia.";
  return "Boa composicao. Mantenha a senha exclusiva e fora de listas publicas.";
}
