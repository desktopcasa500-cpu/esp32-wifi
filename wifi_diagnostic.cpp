#include "wifi_diagnostic.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "settings.h"
#include "ui.h"

namespace {
long tcpLatency(const IPAddress& host, uint16_t port, uint32_t timeoutMs) {
  WiFiClient client;
  const uint32_t started = millis();
  const bool ok = client.connect(host, port, timeoutMs);
  const long elapsed = static_cast<long>(millis() - started);
  client.stop();
  return ok ? elapsed : -1;
}

String latencyText(long value) {
  return value < 0 ? "FAIL" : String(value) + " ms";
}
}

void runWifiDiagnostic(const String& ssid, const String& password) {
  if (ssid.length() == 0) {
    uiMessage("SSID vazio.", "WIFI / DIAGNOSTIC");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false);
  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / DIAGNOSTIC");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Connecting...", 8, 50, 1);

  WiFi.begin(ssid.c_str(), password.c_str());
  const uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && (uint32_t)(millis() - started) < 12000U) {
    if (uiReadInput() == BE_BACK) {
      WiFi.disconnect(false);
      return;
    }
    delay(80);
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(false);
    uiMessage("Falha ao conectar.\nVerifique SSID e senha.", "WIFI / ERROR");
    return;
  }

  long gateway = -1;
  for (uint16_t i = 0; i < settings.pingsPerDiagnostic; ++i) {
    const long result = tcpLatency(WiFi.gatewayIP(), 80, settings.diagnosticTimeoutMs);
    if (result >= 0 && (gateway < 0 || result < gateway)) gateway = result;
    delay(50);
  }

  IPAddress resolved;
  const uint32_t dnsStart = millis();
  const bool dnsOk = WiFi.hostByName("connectivitycheck.gstatic.com", resolved);
  const long dnsMs = dnsOk ? static_cast<long>(millis() - dnsStart) : -1;

  int httpCode = -1;
  long httpMs = -1;
  HTTPClient http;
  http.setConnectTimeout(settings.diagnosticTimeoutMs);
  http.setTimeout(settings.diagnosticTimeoutMs);
  if (http.begin("http://connectivitycheck.gstatic.com/generate_204")) {
    const uint32_t httpStart = millis();
    httpCode = http.GET();
    httpMs = static_cast<long>(millis() - httpStart);
    http.end();
  }

  const bool internet = httpCode == 204 || (httpCode >= 200 && httpCode < 300);
  const bool healthy = internet && gateway >= 0 && gateway <= 150 && dnsOk;

  String report;
  report.reserve(260);
  report += "IP: "; report += WiFi.localIP().toString();
  report += "\nGateway: "; report += WiFi.gatewayIP().toString();
  report += "\nGateway TCP: "; report += latencyText(gateway);
  report += "\nDNS: "; report += dnsOk ? "OK " + resolved.toString() : "FAIL";
  report += "\nDNS time: "; report += latencyText(dnsMs);
  report += "\nHTTP 204: "; report += String(httpCode);
  report += "\nHTTP time: "; report += latencyText(httpMs);
  report += "\n\nStatus: "; report += healthy ? "OK" : "CHECK NETWORK";

  WiFi.disconnect(false);
  uiMessage(report, "WIFI / RESULT");
}
