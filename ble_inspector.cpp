#include "ble_inspector.h"
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
#include "ui.h"

static String propertyText(BLERemoteCharacteristic* c) {
  String p;
  if (c->canRead()) p += "R";
  if (c->canWrite()) p += p.length() ? "/W" : "W";
  if (c->canNotify()) p += p.length() ? "/N" : "N";
  return p.length() ? p : "-";
}

static String hexPreview(const String& value) {
  static const char* hex = "0123456789ABCDEF";
  const int count = min(value.length(), (unsigned int)18);
  String out;
  for (int i = 0; i < count; ++i) {
    const uint8_t b = (uint8_t)value[i];
    if (i) out += ' ';
    out += hex[b >> 4]; out += hex[b & 0x0F];
  }
  if ((int)value.length() > count) out += "...";
  return out;
}

void bleInspectDevice(const String& address) {
  if (address.length() < 10) {
    uiMessage("Nenhum alvo BLE selecionado.", "BLE / GATT");
    return;
  }

  BLEDevice::init("");
  BLEClient* client = BLEDevice::createClient();
  if (!client) {
    uiMessage("Falha ao criar cliente BLE.", "BLE / GATT");
    return;
  }

  tft.fillScreen(TFT_BLACK);
  uiHeader("BLE / GATT", "LIVE");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Connecting...", 8, 48);

  if (!client->connect(BLEAddress(address.c_str()))) {
    delete client;
    uiMessage("Nao foi possivel conectar.\n\nConfirme disponibilidade e autorizacao do dispositivo.", "BLE / GATT");
    return;
  }

  auto* services = client->getServices();
  if (!services) {
    client->disconnect();
    delete client;
    uiMessage("Nenhum servico GATT encontrado.", "BLE / GATT");
    return;
  }

  String report = "Endereco: " + address +
                  "\nServicos: " + String(services->size()) +
                  "\nUptime: " + String(millis() / 1000) + " s\n\n";

  uint8_t serviceCount = 0;
  for (const auto& entry : *services) {
    if (serviceCount++ >= 5) {
      report += "... mais servicos\n";
      break;
    }
    BLERemoteService* service = entry.second;
    report += "S " + String(entry.first.c_str()) + "\n";
    auto* chars = service->getCharacteristics();
    if (!chars) continue;

    uint8_t charCount = 0;
    for (const auto& centry : *chars) {
      if (charCount++ >= 5) {
        report += "  ... mais chars\n";
        break;
      }
      BLERemoteCharacteristic* c = centry.second;
      report += "  C " + String(centry.first.c_str()) +
                " [" + propertyText(c) + "]";
      if (c->canRead()) {
        const String value = c->readValue();
        report += "\n    " + hexPreview(value);
      }
      report += "\n";
    }
  }

  client->disconnect();
  delete client;
  uiMessage(report, "BLE / GATT RESULT");
}
