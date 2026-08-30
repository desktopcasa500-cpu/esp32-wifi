#include "ble_inspector.h"
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
#include "ui.h"

static String propertyText(BLERemoteCharacteristic* characteristic) {
  String p;
  if (characteristic->canRead()) p += "R";
  if (characteristic->canWrite()) p += p.length() ? "/W" : "W";
  if (characteristic->canNotify()) p += p.length() ? "/N" : "N";
  return p.length() ? p : "-";
}

static String hexPreview(const std::string& value) {
  String out;
  const size_t count = min<size_t>(value.length(), 18);
  static const char* hex = "0123456789ABCDEF";
  for (size_t i = 0; i < count; ++i) {
    const uint8_t c = static_cast<uint8_t>(value[i]);
    if (i) out += ' ';
    out += hex[c >> 4];
    out += hex[c & 0x0F];
  }
  if (value.length() > count) out += "...";
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
  uiHeader("BLE / GATT");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Connecting...", 8, 48);

  const BLEAddress target(address.c_str());
  if (!client->connect(target)) {
    uiMessage("Nao foi possivel conectar.\n\nConfirme que o dispositivo esta disponivel e autorizado.", "BLE / GATT");
    delete client;
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
                  "\nServicos: " + String(services->size()) + "\n\n";
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
      if (charCount++ >= 4) {
        report += "  ... mais chars\n";
        break;
      }
      BLERemoteCharacteristic* c = centry.second;
      report += "  C " + String(centry.first.c_str()) +
                " [" + propertyText(c) + "]";
      if (c->canRead()) {
        try {
          std::string value = c->readValue();
          report += "\n    " + hexPreview(value);
        } catch (...) {
          report += "\n    read failed";
        }
      }
      report += "\n";
    }
  }

  client->disconnect();
  delete client;
  uiMessage(report, "BLE / GATT RESULT");
}
