#pragma once
#include <Arduino.h>

// Base compacta de fabricantes mais comuns. Pode ser regenerada por tools/update_oui.py.
struct OuiEntry {
  uint32_t prefix;
  const char* name;
};

static const OuiEntry OUI_DB[] PROGMEM = {
  {0x001CB3, "Apple"}, {0x3C5AB4, "Apple"}, {0xF0D2F1, "Apple"},
  {0x001A11, "Google"}, {0xF4F5D8, "Google"}, {0xC0EEF4, "Samsung"},
  {0x002339, "Samsung"}, {0xA4D1D2, "Xiaomi"}, {0x6496B0, "Xiaomi"},
  {0xB827EB, "Raspberry Pi"}, {0xDC4F22, "Raspberry Pi"},
  {0x00155D, "Microsoft"}, {0x7C1E52, "Intel"}, {0x3CECEF, "Intel"},
  {0x001B21, "Intel"}, {0x001E65, "Cisco"}, {0x0023AE, "Cisco"},
  {0x001CF0, "TP-Link"}, {0x50C7BF, "TP-Link"}, {0xE894F6, "TP-Link"},
  {0xC83A35, "Huawei"}, {0x001E10, "Hewlett-Packard"},
  {0xF8E61A, "Dell"}, {0xA4BB6D, "ASUS"}, {0x2C56DC, "ASUS"},
  {0x001E8C, "Netgear"}, {0x6CB0CE, "Netgear"}, {0x001C10, "D-Link"},
  {0xB8A386, "D-Link"}, {0x001A2B, "Acer"}, {0x001F16, "Sony"}
};

static String ouiVendor(const String& bssid) {
  if (bssid.length() < 8) return "Unknown";
  uint32_t prefix = 0;
  uint8_t digits = 0;
  for (size_t i = 0; i < bssid.length() && digits < 6; ++i) {
    const char c = bssid[i];
    if (c == ':' || c == '-') continue;
    uint8_t v;
    if (c >= '0' && c <= '9') v = c - '0';
    else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
    else return "Unknown";
    prefix = (prefix << 4) | v;
    ++digits;
  }
  if (digits != 6) return "Unknown";
  for (size_t i = 0; i < sizeof(OUI_DB) / sizeof(OUI_DB[0]); ++i) {
    if (prefix == pgm_read_dword(&OUI_DB[i].prefix)) return String(pgm_read_ptr(&OUI_DB[i].name));
  }
  return "Unknown";
}
