#include "channel_optimizer.h"
#include <WiFi.h>
#include "ui.h"
ChannelScore optimizeChannels(){
  ChannelScore best{1,-1,0}; int counts[14]={};
  WiFi.mode(WIFI_STA); int n=WiFi.scanNetworks(false,true);
  for(int i=0;i<n;i++){ int ch=WiFi.channel(i); if(ch>=1&&ch<=13) counts[ch]++; }
  for(int ch=1;ch<=13;ch++){
    int penalty=counts[ch]*12;
    for(int other=1;other<=13;other++) if(other!=ch && abs(other-ch)<5) penalty+=counts[other]*(5-abs(other-ch))*2;
    int score=max(0,100-penalty);
    if((ch==1||ch==6||ch==11)) score=min(100,score+8);
    if(score>best.score) best={(uint8_t)ch,score,(uint8_t)counts[ch]};
  }
  WiFi.scanDelete(); return best;
}
void showChannelOptimizer(){
  ChannelScore b=optimizeChannels(); uiMessage("Melhor canal estimado: CH "+String(b.channel)+"\nScore: "+String(b.score)+"/100\nRedes no canal: "+String(b.networks)+"\n\nA recomendação considera redes detectadas e sobreposição. Refaça a análise após mudanças no ambiente.");
}
