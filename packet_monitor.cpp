#include "packet_monitor.h"
#include <WiFi.h>
#include "esp_wifi.h"

namespace {
volatile uint32_t totalFrames = 0;
volatile uint32_t beaconFrames = 0;
volatile uint32_t probeFrames = 0;
volatile uint32_t dataFrames = 0;
volatile uint32_t mgmtFrames = 0;
volatile uint32_t controlFrames = 0;
volatile uint32_t windowFrames = 0;
volatile bool active = false;
uint32_t ppsWindowStarted = 0;
uint16_t lastPps = 0;

void IRAM_ATTR onPacket(void* buffer, wifi_promiscuous_pkt_type_t type) {
  if (!active || buffer == nullptr) return;

  ++totalFrames;
  ++windowFrames;

  if (type == WIFI_PKT_MGMT) {
    ++mgmtFrames;
    wifi_promiscuous_pkt_t* packet = static_cast<wifi_promiscuous_pkt_t*>(buffer);
    if (packet->rx_ctrl.sig_len < 2) return;

    const uint16_t frameControl =
      static_cast<uint16_t>(packet->payload[0]) |
      (static_cast<uint16_t>(packet->payload[1]) << 8);
    const uint8_t subtype = static_cast<uint8_t>((frameControl >> 4) & 0x0F);

    if (subtype == 8) ++beaconFrames;
    else if (subtype == 4 || subtype == 5) ++probeFrames;
  } else if (type == WIFI_PKT_DATA) {
    ++dataFrames;
  } else if (type == WIFI_PKT_CTRL) {
    ++controlFrames;
  }
}
}

void packetMonitorStart(uint8_t channel) {
  packetMonitorStop();

  totalFrames = 0;
  beaconFrames = 0;
  probeFrames = 0;
  dataFrames = 0;
  mgmtFrames = 0;
  controlFrames = 0;
  windowFrames = 0;
  lastPps = 0;
  ppsWindowStarted = millis();

  WiFi.mode(WIFI_STA);
  if (channel >= 1 && channel <= 13) {
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  }

  active = true;
  esp_wifi_set_promiscuous_rx_cb(onPacket);
  esp_wifi_set_promiscuous(true);
}

void packetMonitorStop() {
  active = false;
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);
}

uint32_t packetCount() {
  return totalFrames;
}

PacketStats packetMonitorStats() {
  if (active && (uint32_t)(millis() - ppsWindowStarted) >= 1000U) {
    lastPps = static_cast<uint16_t>(min<uint32_t>(windowFrames, 65535U));
    windowFrames = 0;
    ppsWindowStarted = millis();
  }

  PacketStats stats;
  stats.total = totalFrames;
  stats.beacon = beaconFrames;
  stats.probe = probeFrames;
  stats.data = dataFrames;
  stats.mgmt = mgmtFrames;
  stats.control = controlFrames;
  stats.pps = lastPps;
  return stats;
}
