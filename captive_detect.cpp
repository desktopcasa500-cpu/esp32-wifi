#include "captive_detect.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "settings.h"

static String classifyPortal(const String& location) {
  String s = location;
  s.toLowerCase();
  if (s.indexOf("meraki") >= 0) return "Cisco Meraki";
  if (s.indexOf("mikrotik") >= 0 || s.indexOf("login") >= 0 && s.indexOf("router") >= 0) return "MikroTik / Router";
  if (s.indexOf("fortinet") >= 0 || s.indexOf("fortigate") >= 0) return "Fortinet";
  if (s.indexOf("paloalto") >= 0 || s.indexOf("palo-alto") >= 0) return "Palo Alto";
  if (s.indexOf("hotel") >= 0) return "Hotel / Guest";
  return "Portal nao identificado";
}

bool detectCaptivePortal(const String& ssid, const String& password, String& report) {
  report = "";
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false);
  WiFi.begin(ssid.c_str(), password.c_str());

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) delay(50);
  if (WiFi.status() != WL_CONNECTED) {
    report = "Falha ao conectar a rede.";
    return false;
  }

  HTTPClient http;
  http.setConnectTimeout(settings.diagnosticTimeoutMs);
  http.setTimeout(settings.diagnosticTimeoutMs);
  http.setFollowRedirects(HTTPC_NO_FOLLOW_REDIRECTS);

  const uint32_t t0 = millis();
  int code = -1;
  String location;
  if (http.begin("http://connectivitycheck.gstatic.com/generate_204")) {
    code = http.GET();
    location = http.getLocation();
    http.end();
  }
  const uint32_t elapsed = millis() - t0;

  IPAddress dnsIP;
  const bool dnsOk = WiFi.hostByName("connectivitycheck.gstatic.com", dnsIP);
  const bool redirected = code == 301 || code == 302 || code == 303 || code == 307 || code == 308;
  const bool captive = redirected || (code >= 200 && code < 300 && code != 204);

  report += "SSID: " + ssid;
  report += "\nHTTP: " + String(code);
  report += "\nTempo: " + String(elapsed) + " ms";
  report += "\nDNS: " + String(dnsOk ? "OK " + dnsIP.toString() : "FAIL");

  if (redirected) {
    report += "\nRedirect: " + location;
    report += "\nTipo: " + classifyPortal(location);
  } else if (captive) {
    report += "\nPossivel portal cativo / interceptacao HTTP.";
  } else if (code == 204) {
    report += "\nResultado: acesso direto, sem portal detectado.";
  } else {
    report += "\nResultado: inconclusivo.";
  }

  WiFi.disconnect(true);
  return captive;
}
