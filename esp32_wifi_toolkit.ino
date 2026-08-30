#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "settings.h"
#include "ui.h"
#include "buttons.h"
#include "wifi_analyzer.h"
#include "wifi_channel_scan.h"
#include "wifi_signal_meter.h"
#include "wifi_spectrum.h"
#include "wifi_hidden_detect.h"
#include "network_detail.h"
#include "channel_optimizer.h"
#include "wifi_diagnostic.h"
#include "packet_monitor.h"
#include "deauth_detect.h"
#include "bt_scanner.h"
#include "ble_inspector.h"
#include "ble_proximity.h"
#include "ble_beacon.h"
#include "ble_advertiser.h"
#include "captive_detect.h"
#include "password_strength.h"
static int menuIndex=0;
static const String menu[]={"WiFi Analyzer","Channel Scanner","Spectrum 2.4G","Hidden Networks","Channel Optimizer","Packet Monitor","Deauth Detector","BLE Scanner","BLE Beacons","BLE GATT","BLE Proximity","BLE Advertiser","Password Strength","System Info","Settings","About"};
static const size_t menuCount=sizeof(menu)/sizeof(menu[0]);
void showSystem(){uiMessage("ESP32 WiFi Toolkit v9.0\n\nCPU: "+String(getCpuFrequencyMhz())+" MHz\nFree heap: "+String(ESP.getFreeHeap())+" bytes\nFlash: "+String(ESP.getFlashChipSize()/1024/1024)+" MB\nUptime: "+String(millis()/1000)+" s");}
void showSettings(){uiMessage("CONFIG\n\nWiFi scan: "+String(settings.wifiScanMs)+" ms\nBLE scan: "+String(settings.bleScanMs)+" ms\nBrilho: "+String(settings.brightness)+"\nCanal: "+String(settings.defaultChannel)+"\nHidden scan: "+String(settings.hiddenScan?"ON":"OFF")+"\nNome: "+settings.deviceName);}
void showAbout(){uiMessage("ESP32 WiFi Toolkit v9.0\n\nDiagnostico WiFi/BLE e observacao passiva.\n\nSem ataques, captura de credenciais ou interferencia intencional.");}
void render(){uiMenu(menu,menuCount,menuIndex);}
void selectMenu(){switch(menuIndex){case 0:wifiScan(settings.hiddenScan);break;case 1:showChannelScan();break;case 2:showWifiSpectrum();break;case 3:showHiddenNetworks();break;case 4:showChannelOptimizer();break;case 5:packetMonitorStart(settings.defaultChannel);delay(5000);uiMessage("Frames observados: "+String(packetCount())+"\nModo passivo");packetMonitorStop();break;case 6:deauthDetectorStart(settings.defaultChannel);delay(5000);deauthDetectorStop();uiMessage("Monitoramento passivo concluido.\nCanal: "+String(settings.defaultChannel));break;case 7:bleScan(settings.bleScanMs/1000);break;case 8:beaconDetect(settings.bleScanMs/1000);break;case 9:uiMessage("GATT Inspector\n\nUse bleInspectDevice(address) para um dispositivo autorizado.");break;case 10:uiMessage("BLE Proximity\n\nUse bleProximityMonitor(address) para um dispositivo autorizado.");break;case 11:bleAdvertiserStart(0,500);uiMessage("BLE Advertiser ativo\nPerfil: Generico\nIntervalo: 500 ms");bleAdvertiserStop();break;case 12:{PasswordReport r=analyzePassword("exemplo-senha-123");uiMessage("Analise offline\nScore: "+String(r.score)+"/100\nEntropia: "+String(r.entropy,1)+" bits\nComum: "+String(r.common?"sim":"nao")+"\nSequencia: "+String(r.hasSequence?"sim":"nao")+"\n\n"+passwordAdvice(r));}break;case 13:showSystem();break;case 14:showSettings();break;case 15:showAbout();break;}delay(400);render();}
void setup(){Serial.begin(115200);settingsLoad();uiBegin();buttonsBegin();WiFi.mode(WIFI_STA);WiFi.disconnect();render();}
void loop(){switch(buttonsRead()){case BE_PREV:menuIndex=(menuIndex+menuCount-1)%menuCount;render();break;case BE_NEXT:menuIndex=(menuIndex+1)%menuCount;render();break;case BE_SELECT:selectMenu();break;case BE_BACK:render();break;default:break;}delay(10);}
