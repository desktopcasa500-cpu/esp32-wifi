#!/usr/bin/env python3
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

REQUIRED = {
    "esp32_wifi_toolkit.ino",
    "config.h", "settings.h", "settings.cpp", "buttons.h", "buttons.cpp",
    "touch.h", "touch.cpp", "ui.h", "ui.cpp",
    "wifi_analyzer.h", "wifi_analyzer.cpp",
    "wifi_channel_scan.h", "wifi_channel_scan.cpp",
    "wifi_signal_meter.h", "wifi_signal_meter.cpp",
    "wifi_spectrum.h", "wifi_spectrum.cpp",
    "wifi_hidden_detect.h", "wifi_hidden_detect.cpp",
    "network_detail.h", "network_detail.cpp",
    "channel_optimizer.h", "channel_optimizer.cpp",
    "wifi_diagnostic.h", "wifi_diagnostic.cpp",
    "packet_monitor.h", "packet_monitor.cpp",
    "deauth_detect.h", "deauth_detect.cpp",
    "bt_scanner.h", "bt_scanner.cpp",
    "ble_inspector.h", "ble_inspector.cpp",
    "ble_proximity.h", "ble_proximity.cpp",
    "ble_beacon.h", "ble_beacon.cpp",
    "ble_advertiser.h", "ble_advertiser.cpp",
    "captive_detect.h", "captive_detect.cpp",
    "password_strength.h", "password_strength.cpp",
    "User_Setup.h",
}

errors: list[str] = []
for name in sorted(REQUIRED):
    if not (ROOT / name).exists():
        errors.append(f"missing file: {name}")

for path in ROOT.glob("*.cpp"):
    text = path.read_text(encoding="utf-8", errors="replace")

    # Arduino-ESP32 2.0.16 returns BLEScanResults by value.
    if re.search(r"BLEScanResults\s*\*\s*\w+\s*=\s*\w+->start\(", text):
        errors.append(f"BLEScan::start pointer form found: {path.name}")

    if re.search(r"\btry\s*\{", text):
        errors.append(f"exception syntax found in embedded module: {path.name}")

    if "wifi_promiscuous_pkt_t" in text:
        for match in re.finditer(r"payload\[0\]", text):
            prefix = text[max(0, match.start() - 600): match.start()]
            if "sig_len" not in prefix:
                errors.append(f"promiscuous payload access lacks nearby sig_len guard: {path.name}")

    # TFT_eSPI 2.x requires an explicit font number for drawRightString().
    for match in re.finditer(r"drawRightString\s*\(([^\n;]*)\)", text):
        if match.group(1).count(",") < 3:
            errors.append(f"drawRightString has fewer than 4 args: {path.name}")

main_path = ROOT / "esp32_wifi_toolkit.ino"
main = main_path.read_text(encoding="utf-8", errors="replace")
if "uiMenu(MENU, MENU_COUNT" not in main:
    errors.append("main does not use the consolidated UI menu API")
if "WiFi.disconnect(true)" in main:
    errors.append("main erases WiFi configuration with disconnect(true)")

ui = (ROOT / "ui.cpp").read_text(encoding="utf-8", errors="replace")
if "void uiMenu(const char* const items[]" not in ui:
    errors.append("uiMenu is not using the const char* menu representation")

print(f"checked: {len(REQUIRED)} project files")
if errors:
    print("FAIL")
    for error in errors:
        print(f"- {error}")
    raise SystemExit(1)

print("OK: source consistency checks passed for Arduino-ESP32 Core 2.0.16")
