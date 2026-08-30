#include "ble_inspector.h"
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
#include <string>
#include "ui.h"

namespace {
String properties(BLERemoteCharacteristic* characteristic) {
  String value;
  if (characteristic->canRead()) value += "R";
  if (characteristic->canWrite()) value += value.length() ? "/W" : "W";
  if (characteristic->canNotify()) value += value.length() ? "/N" : "N";
  return value.length() ? value : "-";
}

String bytesPreview(const std::string& value) {
  static const char* HEX = "0123456789ABCDEF";
  const size_t count = min(value.size(), static_cast<size_t>(12));
  String out;
  for (size_t i = 0; i < count; ++i) {
    if (i) out += ' ';
    const uint8_t b = static_cast<uint8_t>(value[i]);
    out += HEX[b >> 4];
    out += HEX[b & 0x0F];
  }
  if (value.size() > count) out += " ...";
  return out;
}
}

void bleInspectDevice(const String& address) {
  if (address.length() != 17) {
    uiMessage("Nenhum alvo BLE valido.", "BLE / GATT");
    return;
  }

  BLEDevice::init("");
  BLEClient* client = BLEDevice::createClient();
  if (client == nullptr) {
    uiMessage("Nao foi possivel criar o cliente BLE.", "BLE / GATT");
    return;
  }

  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / GATT", "CONNECT");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(address, 8, 50, 1);

  if (!client->connect(BLEAddress(address.c_str()))) {
    delete client;
    uiMessage("Conexao falhou.\n\nUse apenas dispositivos que voce pode testar.", "BLE / GATT");
    return;
  }

  BLERemoteService* firstService = nullptr;
  const std::map<std::string, BLERemoteService*>* services = client->getServices();
  if (services == nullptr || services->empty()) {
    client->disconnect();
    delete client;
    uiMessage("Nenhum servico GATT encontrado.", "BLE / GATT");
    return;
  }

  String report;
  report.reserve(1000);
  report += "Address: ";
  report += address;
  report += "\nServices: ";
  report += String(services->size());

  uint8_t serviceCount = 0;
  for (std::map<std::string, BLERemoteService*>::const_iterator it = services->begin();
       it != services->end() && serviceCount < 5; ++it, ++serviceCount) {
    if (firstService == nullptr) firstService = it->second;
    report += "\nS ";
    report += it->first.c_str();

    const std::map<std::string, BLERemoteCharacteristic*>* chars = it->second->getCharacteristics();
    if (chars == nullptr) continue;

    uint8_t charCount = 0;
    for (std::map<std::string, BLERemoteCharacteristic*>::const_iterator ci = chars->begin();
         ci != chars->end() && charCount < 4; ++ci, ++charCount) {
      report += "\n C ";
      report += ci->first.c_str();
      report += " [";
      report += properties(ci->second);
      report += "]";
      if (ci->second->canRead()) {
        const std::string raw = ci->second->readValue();
        report += "\n  ";
        report += bytesPreview(raw);
      }
    }
  }

  client->disconnect();
  delete client;
  uiMessage(report, "BLE / GATT RESULT");
}
