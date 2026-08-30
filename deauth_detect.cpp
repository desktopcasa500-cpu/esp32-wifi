#include "deauth_detect.h"
#include <WiFi.h>
#include "esp_wifi.h"

namespace {
volatile uint32_t deauthCount = 0;
volatile uint32_t disassocCount = 0;
volatile int lastRssi = -127;
volatile uint8_t lastChannel = 0;
volatile bool active = false;

void IRAM_ATTR onPacket(void* buffer, wifi_promiscuous_pkt_type_t type) {
  if (!active || buffer == nullptr || type != WIFI_PKT_MGMT) return;

  wifi_promiscuous_pkt_t* packet = static_cast<wifi_promiscuous_pkt_t*>(buffer);
  if (packet->rx_ctrl.sig_len < 2) return;

  const uint16_t frameControl =
    static_cast<uint16_t>(packet->payload[0]) |
    (static_cast<uint16_t>(packet->payload[1]) << 8);
  const uint8_t frameType = static_cast<uint8_t>((frameControl >> 2) & 0x03);
  const uint8_t subtype = static_cast<uint8_t>((frameControl >> 4) & 0x0F);

  if (frameType != 0) return;
  if (subtype == 12) ++deauthCount;
  else if (subtype == 10) ++disassocCount;
  else return;
  lastRssi = packet->rx_ctrl.rssi;
}
}

void deauthDetectorStart(uint8_t channel) {
  deauthDetectorStop();
  deauthCount = 0;
  disassocCount = 0;
  lastRssi = -127;
  lastChannel = (channel >= 1 && channel <= 13) ? channel : 0;

  WiFi.mode(WIFI_STA);
  if (lastChannel) esp_wifi_set_channel(lastChannel, WIFI_SECOND_CHAN_NONE);

  esp_wifi_set_promiscuous_rx_cb(onPacket);
  active = true;
  esp_wifi_set_promiscuous(true);
}

void deauthDetectorStop() {
  active = false;
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);
}

uint32_t deauthEvents() { return deauthCount; }
uint32_t disassocEvents() { return disassocCount; }
int deauthLastRssi() { return lastRssi; }
uint8_t deauthChannel() { return lastChannel; }
