#!/usr/bin/env python3
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

required = {
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
for name in sorted(required):
    if not (ROOT / name).exists():
        errors.append(f"missing file: {name}")

pointer_scan = []
try_blocks = []
unsafe_callbacks = []
right_string_short = []
for path in ROOT.glob("*.cpp"):
    text = path.read_text(encoding="utf-8", errors="replace")

    # Arduino-ESP32 2.0.x exposes BLEScan::start() as a value return.
    if re.search(r"BLEScanResults\s*\*\s*\w+\s*=\s*\w+->start\(", text):
        pointer_scan.append(path.name)

    if re.search(r"\btry\s*\{", text):
        try_blocks.append(path.name)

    if "wifi_promiscuous_pkt_t" in text:
        for match in re.finditer(r"pkt->payload\[0\]", text):
            prefix = text[max(0, match.start() - 500): match.start()]
            if "sig_len" not in prefix:
                unsafe_callbacks.append(path.name)

    # TFT_eSPI used by this project requires the explicit font argument in
    # drawRightString() calls for the installed 2.x library.
    for match in re.finditer(r"drawRightString\s*\(([^\n;]*)\)", text):
        args = match.group(1)
        if args.count(",") < 3:
            right_string_short.append(path.name)

for name in pointer_scan:
    errors.append(f"BLE scan result uses pointer form incompatible with ESP32 Core 2.0.16: {name}")
for name in try_blocks:
    errors.append(f"exception handling found in embedded BLE module: {name}")
for name in sorted(set(unsafe_callbacks)):
    errors.append(f"promiscuous payload access without nearby sig_len check: {name}")
for name in sorted(set(right_string_short)):
    errors.append(f"drawRightString call has fewer than 4 arguments: {name}")

main = (ROOT / "esp32_wifi_toolkit.ino").read_text(encoding="utf-8", errors="replace")
for include in re.findall(r'#include\s+["<]([^">]+)[">]', main):
    if include.endswith((".h", ".hpp")) and not include.startswith(("Arduino", "WiFi", "TFT", "BLE", "HTTP", "Preferences")):
        if not (ROOT / include).exists():
            errors.append(f"main include not found in project: {include}")

print(f"checked: {len(required)} required project files")
if errors:
    print("FAIL")
    for error in errors:
        print(f"- {error}")
    raise SystemExit(1)

print("OK: source consistency checks passed for Arduino-ESP32 2.0.16")
