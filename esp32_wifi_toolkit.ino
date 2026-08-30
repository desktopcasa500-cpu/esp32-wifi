#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "settings.h"
#include "ui.h"
#include "buttons.h"
#include "wifi_analyzer.h"
#include "channel_optimizer.h"
#include "wifi_diagnostic.h"
#include "bt_scanner.h"
#include "password_strength.h"

static int menuIndex=0;
static const String menu[]={"WiFi Analyzer","Channel Optimizer","BLE Scanner","WiFi Diagnostics","Password Strength","System Info","Settings","About"};
static const size_t menuCount=sizeof(menu)/sizeof(menu[0]);

void showSystem(){
  uiMessage("ESP32 WiFi Toolkit v9.0\n\nChip: ESP32\nCPU: "+String(getCpuFrequencyMhz())+" MHz\nFree heap: "+String(ESP.getFreeHeap())+" bytes\nFlash: "+String(ESP.getFlashChipSize()/1024/1024)+" MB\nUptime: "+String(millis()/1000)+" s");
}
void showAbout(){ uiMessage("ESP32 WiFi Toolkit v9.0\n\nFerramentas de diagnóstico WiFi/BLE e observação passiva.\n\nSomente use em redes e dispositivos próprios ou autorizados.\nAtaques, captura de credenciais e interferência intencional não fazem parte do firmware."); }
void render(){ uiMenu(menu,menuCount,menuIndex); }
void selectMenu(){
  switch(menuIndex){
    case 0: wifiScan(settings.hiddenScan); break;
    case 1: showChannelOptimizer(); break;
    case 2: bleScan(settings.bleScanMs/1000); break;
    case 3: uiMessage("Diagnóstico: selecione uma rede e forneça a senha através da interface de configuração.\n\nA API runWifiDiagnostic() realiza somente testes de conectividade autorizados."); break;
    case 4: { PasswordReport r=analyzePassword("exemplo-senha-123"); uiMessage("Exemplo de análise offline\nScore: "+String(r.score)+"/100\nEntropia estimada: "+String(r.entropy,1)+" bits\nComum: "+String(r.common?"sim":"não")+"\nSequência: "+String(r.hasSequence?"sim":"não")+"\n\n"+passwordAdvice(r)); } break;
    case 5: showSystem(); break;
    case 6: uiMessage("Configurações persistentes\n\nWiFi scan: "+String(settings.wifiScanMs)+" ms\nBLE scan: "+String(settings.bleScanMs)+" ms\nBrilho: "+String(settings.brightness)+"\nCanal padrão: "+String(settings.defaultChannel)+"\nNome: "+settings.deviceName+"\n\nUse os botões para navegar. Edição detalhada pode ser adicionada à tela Settings."); break;
    case 7: showAbout(); break;
  }
  delay(500); render();
}
void setup(){ Serial.begin(115200); settingsLoad(); uiBegin(); buttonsBegin(); WiFi.mode(WIFI_STA); WiFi.disconnect(); render(); }
void loop(){
  switch(buttonsRead()){
    case BE_PREV: menuIndex=(menuIndex+menuCount-1)%menuCount; render(); break;
    case BE_NEXT: menuIndex=(menuIndex+1)%menuCount; render(); break;
    case BE_SELECT: selectMenu(); break;
    case BE_BACK: render(); break;
    default: break;
  }
  delay(10);
}
