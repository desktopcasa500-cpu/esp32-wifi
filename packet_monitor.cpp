#include "packet_monitor.h"
#include <WiFi.h>
#include "esp_wifi.h"
static volatile uint32_t total=0; static bool active=false;
static void rx(void*,wifi_promiscuous_pkt_type_t){ if(active) total++; }
void packetMonitorStart(uint8_t channel){ total=0; WiFi.mode(WIFI_STA); if(channel>=1&&channel<=13) esp_wifi_set_channel(channel,WIFI_SECOND_CHAN_NONE); esp_wifi_set_promiscuous_rx_cb(rx); esp_wifi_set_promiscuous(true); active=true; }
void packetMonitorStop(){ active=false; esp_wifi_set_promiscuous(false); esp_wifi_set_promiscuous_rx_cb(nullptr); }
uint32_t packetCount(){ return total; }
