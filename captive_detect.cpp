#include "captive_detect.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "settings.h"

namespace {
String classifyPortal(const String& location) {
  String s = location;
  s.toLowerCase();
  if (s.indexOf("meraki") >= 0) return "Cisco Meraki";
  if (s.indexOf("mikrotik") >= 0) return "MikroTik";
  if (s.indexOf("fortinet") >= 0 || s.indexOf("fortigate") >= 0) return "Fortinet";
  if (s.indexOf("paloalto") >= 0 || s.indexOf("palo-alto") >= 0) return "Palo Alto";
  if (s.indexOf("hotel") >= 0) return "Hotel / Guest";
  return "Unknown portal";
}
}

bool detectCaptivePortal(const String& ssid, const String& password, String& report) {
  report = "";
  if (!ssid.length()) {
    report = "SSID vazio.";
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false);
  WiFi.begin(ssid.c_str(), password.c_str());

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (uint32_t)(millis() - start) < 12000U) {
    delay(80);
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(false);
    report = "Falha ao conectar a rede.";
    return false;
  }

  int code = -1;
  long elapsed = -1;
  String location;
  HTTPClient http;
  http.setConnectTimeout(settings.diagnosticTimeoutMs);
  http.setTimeout(settings.diagnosticTimeoutMs);
  http.setFollowRedirects(HTTPC_NO_FOLLOW_REDIRECTS);

  if (http.begin("http://connectivitycheck.gstatic.com/generate_204")) {
    const uint32_t t0 = millis();
    code = http.GET();
    elapsed = static_cast<long>(millis() - t0);
    location = http.getLocation();
    http.end();
  }

  IPAddress dnsIP;
  const bool dnsOk = WiFi.hostByName("connectivitycheck.gstatic.com", dnsIP);
  const bool redirect = code == 301 || code == 302 || code == 303 || code == 307 || code == 308;
  const bool captive = redirect || (code >= 200 && code < 300 && code != 204);

  report.reserve(300);
  report += "HTTP: "; report += String(code);
  report += "\nTime: "; report += String(elapsed < 0 ? 0 : elapsed); report += " ms";
  report += "\nDNS: "; report += dnsOk ? "OK " + dnsIP.toString() : "FAIL";

  if (redirect) {
    report += "\nRedirect: ";
    report += location.length() ? location.substring(0, 120) : "<none>";
    report += "\nType: ";
    report += classifyPortal(location);
  } else if (code == 204) {
    report += "\nResult: direct internet access";
  } else if (captive) {
    report += "\nResult: possible captive portal";
  } else {
    report += "\nResult: inconclusive";
  }

  WiFi.disconnect(false);
  return captive;
}
