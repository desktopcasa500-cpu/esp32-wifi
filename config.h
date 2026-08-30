#pragma once
#include <Arduino.h>

#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_SCLK 18
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST 4
#define TFT_LED 27
#define TOUCH_CLK 14
#define TOUCH_MOSI 13
#define TOUCH_MISO 12
#define TOUCH_CS 5
#define TOUCH_IRQ 21
#define BTN_PREV 32
#define BTN_NEXT 33
#define BTN_SELECT 25
#define BTN_BACK 26

enum AppState { ST_HOME, ST_WIFI, ST_BLE, ST_TOOLS, ST_SETTINGS, ST_WIFI_SCAN, ST_WIFI_CHANNELS, ST_WIFI_SIGNAL, ST_WIFI_DIAG, ST_BLE_SCAN, ST_BLE_BEACONS, ST_BLE_GATT, ST_PASSWORD, ST_ABOUT };
