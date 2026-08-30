#include "deauth_detect.h"
#include <WiFi.h>
#include "esp_wifi.h"

static volatile uint32_t deauthCount = 0;
static volatile uint32_t disassocCount = 0;
static volatile int lastRssi = -127;
static volatile uint8_t lastChannel = 0;
static bool running = false;

static void promiscuous(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!running || !buf || type != WIFI_PKT_MGMT) return;

  wifi_promiscuous_pkt_t* pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
  const uint16_t fc = static_cast<uint16_t>(pkt->payload[0]) |
                      (static_cast<uint16_t>(pkt->payload[1]) << 8);
  const uint8_t subtype = (fc >> 4) & 0x0F;

  if (subtype == 12) ++deauthCount;
  else if (subtype == 10) ++disassocCount;
  else return;

  lastRssi = pkt->rx_ctrl.rssi;
}

void deauthDetectorStart(uint8_t channel) {
  deauthDetectorStop();
  deauthCount = 0;
  disassocCount = 0;
  lastRssi = -127;
  lastChannel = channel;
  WiFi.mode(WIFI_STA);
  if (channel >= 1 && channel <= 13) {
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  }
  esp_wifi_set_promiscuous_rx_cb(promiscuous);
  esp_wifi_set_promiscuous(true);
  running = true;
}

void deauthDetectorStop() {
  running = false;
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);
}

uint32_t deauthEvents() {
  return deauthCount;
}

uint32_t disassocEvents() {
  return disassocCount;
}

int deauthLastRssi() {
  return lastRssi;
}

uint8_t deauthChannel() {
  return lastChannel;
}
