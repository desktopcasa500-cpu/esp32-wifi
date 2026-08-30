#include "wifi_diagnostic.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "settings.h"
#include "ui.h"

static long tcpTest(const IPAddress& ip, uint16_t port, uint32_t timeout) {
  WiFiClient client;
  const uint32_t start = millis();
  const bool ok = client.connect(ip, port, timeout);
  const long elapsed = millis() - start;
  client.stop();
  return ok ? elapsed : -1;
}

static String latencyText(long value) {
  return value < 0 ? "FAIL" : String(value) + " ms";
}

void runWifiDiagnostic(const String& ssid, const String& password) {
  tft.fillScreen(TFT_BLACK);
  uiHeader("WIFI / DIAGNOSTIC");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Connecting...", 8, 48);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false);
  WiFi.begin(ssid.c_str(), password.c_str());

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
    delay(100);
  }

  if (WiFi.status() != WL_CONNECTED) {
    uiMessage("Nao foi possivel conectar.\n\nVerifique SSID e senha.", "WIFI / ERROR");
    return;
  }

  long gatewayLatency = -1;
  long dnsLatency = -1;
  long httpLatency = -1;
  int httpCode = -1;
  bool dnsOk = false;

  for (uint8_t i = 0; i < settings.pingsPerDiagnostic; ++i) {
    const long v = tcpTest(WiFi.gatewayIP(), 80, settings.diagnosticTimeoutMs);
    if (v >= 0 && (gatewayLatency < 0 || v < gatewayLatency)) gatewayLatency = v;
    delay(80);
  }

  const uint32_t dnsStart = millis();
  IPAddress resolved;
  dnsOk = WiFi.hostByName("connectivitycheck.gstatic.com", resolved);
  dnsLatency = dnsOk ? (long)(millis() - dnsStart) : -1;

  HTTPClient http;
  const uint32_t httpStart = millis();
  http.setConnectTimeout(settings.diagnosticTimeoutMs);
  http.setTimeout(settings.diagnosticTimeoutMs);
  if (http.begin("http://connectivitycheck.gstatic.com/generate_204")) {
    httpCode = http.GET();
    httpLatency = millis() - httpStart;
    http.end();
  }

  const bool internetOk = httpCode == 204 || (httpCode >= 200 && httpCode < 400);
  String verdict;
  if (!internetOk) verdict = "Internet: FAIL";
  else if (gatewayLatency > 150) verdict = "Rede: latencia alta";
  else verdict = "Rede: OK";

  uiMessage(
    "SSID: " + ssid +
    "\nIP: " + WiFi.localIP().toString() +
    "\nGateway: " + WiFi.gatewayIP().toString() +
    "\nGateway TCP: " + latencyText(gatewayLatency) +
    "\nDNS resolve: " + String(dnsOk ? "OK " + resolved.toString() : "FAIL") +
    "\nDNS tempo: " + latencyText(dnsLatency) +
    "\nHTTP 204: " + String(httpCode) +
    "\nHTTP tempo: " + latencyText(httpLatency) +
    "\n\n" + verdict,
    "WIFI / RESULT"
  );

  WiFi.disconnect(true);
}
