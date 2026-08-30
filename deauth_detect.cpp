#include "deauth_detect.h"
#include <WiFi.h>
#include "esp_wifi.h"
static bool running=false; static volatile uint32_t frames=0;
static void promiscuous(void*,wifi_promiscuous_pkt_type_t type){ if(running && type==WIFI_PKT_MGMT) frames++; }
void deauthDetectorStart(uint8_t channel){ running=true; frames=0; WiFi.mode(WIFI_STA); if(channel>=1&&channel<=13) esp_wifi_set_channel(channel,WIFI_SECOND_CHAN_NONE); esp_wifi_set_promiscuous_rx_cb(promiscuous); esp_wifi_set_promiscuous(true); }
void deauthDetectorStop(){ running=false; esp_wifi_set_promiscuous(false); esp_wifi_set_promiscuous_rx_cb(nullptr); }
