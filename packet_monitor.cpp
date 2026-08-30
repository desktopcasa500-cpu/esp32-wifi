#include "packet_monitor.h"
#include <WiFi.h>
#include "esp_wifi.h"

static volatile uint32_t totalFrames = 0;
static volatile uint32_t beaconFrames = 0;
static volatile uint32_t probeFrames = 0;
static volatile uint32_t dataFrames = 0;
static volatile uint32_t mgmtFrames = 0;
static volatile uint32_t controlFrames = 0;
static volatile uint32_t windowFrames = 0;
static volatile bool active = false;
static uint32_t windowStart = 0;
static uint16_t lastPps = 0;

static void rxCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!active || !buf) return;

  ++totalFrames;
  ++windowFrames;

  if (type == WIFI_PKT_MGMT) {
    ++mgmtFrames;
    wifi_promiscuous_pkt_t* pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
    if (pkt->rx_ctrl.sig_len < 2) return;

    const uint16_t fc = static_cast<uint16_t>(pkt->payload[0]) |
                        (static_cast<uint16_t>(pkt->payload[1]) << 8);
    const uint8_t subtype = (fc >> 4) & 0x0F;
    if (subtype == 8) ++beaconFrames;
    else if (subtype == 4 || subtype == 5) ++probeFrames;
  } else if (type == WIFI_PKT_DATA) {
    ++dataFrames;
  } else if (type == WIFI_PKT_CTRL) {
    ++controlFrames;
  }
}

void packetMonitorStart(uint8_t channel) {
  packetMonitorStop();
  totalFrames = beaconFrames = probeFrames = dataFrames = mgmtFrames = controlFrames = 0;
  windowFrames = 0;
  lastPps = 0;
  windowStart = millis();

  WiFi.mode(WIFI_STA);
  if (channel >= 1 && channel <= 13) {
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  }

  active = true;
  esp_wifi_set_promiscuous_rx_cb(rxCallback);
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
  if (active && millis() - windowStart >= 1000) {
    lastPps = static_cast<uint16_t>(min<uint32_t>(windowFrames, 65535));
    windowFrames = 0;
    windowStart = millis();
  }

  PacketStats s;
  s.total = totalFrames;
  s.beacon = beaconFrames;
  s.probe = probeFrames;
  s.data = dataFrames;
  s.mgmt = mgmtFrames;
  s.control = controlFrames;
  s.pps = lastPps;
  return s;
}
