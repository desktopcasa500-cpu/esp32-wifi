#include "buttons.h"
#include "config.h"
static uint8_t pins[]={BTN_PREV,BTN_NEXT,BTN_SELECT,BTN_BACK}; static bool last[4]={1,1,1,1}; static uint32_t debounce[4]={};
void buttonsBegin(){ for(uint8_t p:pins) pinMode(p,INPUT_PULLUP); }
ButtonEvent buttonsRead(){ for(int i=0;i<4;i++){ bool now=digitalRead(pins[i]); if(now!=last[i]){ if(millis()-debounce[i]>30){debounce[i]=millis(); last[i]=now; if(!now)return (ButtonEvent)(i+1);}}} return BE_NONE; }
