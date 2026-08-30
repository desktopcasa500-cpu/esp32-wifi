#include "ble_inspector.h"
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
#include <map>
#include <string>
#include "ui.h"

namespace {
String propertyText(BLERemoteCharacteristic* characteristic) {
  String out;
  if (characteristic->canRead()) out += "R";
  if (characteristic->canWrite()) out += out.length() ? "/W" : "W";
  if (characteristic->canNotify()) out += out.length() ? "/N" : "N";
  return out.length() ? out : "-";
}

String hexPreview(const std::string& raw) {
  static const char* HEX = "0123456789ABCDEF";
  const size_t count = min(raw.size(), static_cast<size_t>(12));
  String out;
  for (size_t i = 0; i < count; ++i) {
    if (i) out += ' ';
    const uint8_t b = static_cast<uint8_t>(raw[i]);
    out += HEX[b >> 4];
    out += HEX[b & 0x0F];
  }
  if (raw.size() > count) out += " ...";
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
  if (!client) {
    uiMessage("Falha ao criar o cliente BLE.", "BLE / GATT");
    return;
  }

  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / GATT", "CONNECT");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(address, 8, 50, 1);

  const BLEAddress target(address.c_str());
  if (!client->connect(target)) {
    delete client;
    uiMessage("Nao foi possivel conectar ao dispositivo.", "BLE / GATT");
    return;
  }

  const std::map<std::string, BLERemoteService*>* services = client->getServices();
  if (!services || services->empty()) {
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

  uint8_t serviceShown = 0;
  for (std::map<std::string, BLERemoteService*>::const_iterator it = services->begin();
       it != services->end() && serviceShown < 5; ++it, ++serviceShown) {
    report += "\nS ";
    report += it->first.c_str();

    const std::map<std::string, BLERemoteCharacteristic*>* chars = it->second->getCharacteristics();
    if (!chars) continue;

    uint8_t charShown = 0;
    for (std::map<std::string, BLERemoteCharacteristic*>::const_iterator ci = chars->begin();
         ci != chars->end() && charShown < 4; ++ci, ++charShown) {
      BLERemoteCharacteristic* characteristic = ci->second;
      report += "\n C ";
      report += ci->first.c_str();
      report += " [";
      report += propertyText(characteristic);
      report += "]";

      if (characteristic->canRead()) {
        const std::string raw = characteristic->readValue();
        report += "\n  ";
        report += hexPreview(raw);
      }
    }
  }

  client->disconnect();
  delete client;
  uiMessage(report, "BLE / GATT RESULT");
}
