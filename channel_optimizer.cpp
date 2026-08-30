#include "channel_optimizer.h"
#include <WiFi.h>
#include "ui.h"

ChannelScore optimizeChannels() {
  ChannelScore best{1, 0, 0};
  int counts[14] = {};
  int weighted[14] = {};

  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  const int n = WiFi.scanNetworks(false, true);
  for (int i = 0; i < n; ++i) {
    const int ch = WiFi.channel(i);
    if (ch < 1 || ch > 13) continue;
    ++counts[ch];
    const int rssi = WiFi.RSSI(i);
    weighted[ch] += constrain(rssi + 100, 1, 70);
  }

  for (int ch = 1; ch <= 13; ++ch) {
    int load = weighted[ch];
    for (int other = 1; other <= 13; ++other) {
      if (other == ch) continue;
      const int distance = abs(other - ch);
      if (distance < 5) {
        const int factor = 5 - distance;
        load += (weighted[other] * factor) / 5;
      }
    }

    int score = constrain(100 - load, 0, 100);
    if (ch != 1 && ch != 6 && ch != 11) score = max(0, score - 8);

    if (score > best.score) {
      best.channel = ch;
      best.score = score;
      best.networks = counts[ch];
    }
  }

  WiFi.scanDelete();
  return best;
}

void showChannelOptimizer() {
  const ChannelScore b = optimizeChannels();
  uiMessage(
    "Melhor canal: CH " + String(b.channel) +
    "\nScore: " + String(b.score) + "/100" +
    "\nRedes no canal: " + String(b.networks) +
    "\n\nO score considera RSSI e sobreposicao das redes vistas.\nRefaça a analise depois de alterar o roteador.",
    "WIFI / OPTIMIZER"
  );
}
