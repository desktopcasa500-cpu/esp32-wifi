#include "channel_optimizer.h"
#include <WiFi.h>
#include "ui.h"

namespace {
int channelScore(const int weighted[14], const int counts[14], int channel) {
  int load = weighted[channel];
  for (int other = 1; other <= 13; ++other) {
    if (other == channel) continue;
    const int distance = abs(other - channel);
    if (distance < 5) load += (weighted[other] * (5 - distance)) / 5;
  }

  int score = 100 - load;
  score -= counts[channel] * 3;
  if (channel != 1 && channel != 6 && channel != 11) score -= 8;
  return constrain(score, 0, 100);
}
}

ChannelScore optimizeChannels() {
  ChannelScore best = {1, 0, 0};
  int counts[14] = {0};
  int weighted[14] = {0};

  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  const int total = WiFi.scanNetworks(false, true);
  if (total < 0) return best;

  for (int i = 0; i < total; ++i) {
    const int channel = WiFi.channel(i);
    if (channel < 1 || channel > 13) continue;
    ++counts[channel];
    weighted[channel] += constrain(WiFi.RSSI(i) + 100, 1, 70);
  }

  for (int channel = 1; channel <= 13; ++channel) {
    const int score = channelScore(weighted, counts, channel);
    if (score > best.score) {
      best.channel = channel;
      best.score = score;
      best.networks = counts[channel];
    }
  }

  WiFi.scanDelete();
  return best;
}

void showChannelOptimizer() {
  const ChannelScore result = optimizeChannels();
  String report;
  report.reserve(260);
  report += "Best channel: CH ";
  report += String(result.channel);
  report += "\nScore: ";
  report += String(result.score);
  report += "/100\nNetworks: ";
  report += String(result.networks);
  report += "\n\nUse CH1, CH6 or CH11 when possible.\nScore is based on observed RSSI and overlap.";
  uiMessage(report, "WIFI / OPTIMIZER");
}
