# ESP32 WiFi Toolkit v9.0

Firmware educacional de análise e diagnóstico WiFi/BLE para ESP32.

## Recursos

### WiFi
- Analisador WiFi com RSSI, canal, segurança e fabricante
- Scanner e ranking de canais 2.4 GHz
- Medidor de sinal com histórico RSSI
- Monitor passivo de frames 802.11
- Detalhes completos de redes
- Detector passivo de redes ocultas
- Visualização de espectro 2.4 GHz
- Diagnóstico de conectividade
- Detector passivo de deauth/desassociação
- Detector de portal cativo
- Otimizador de canal
- Analisador offline de força de senha

### Bluetooth / BLE
- Scanner BLE e identificação de fabricantes
- Detecção de iBeacon e Eddystone
- Inspetor GATT para dispositivos autorizados
- Monitor de proximidade por RSSI
- Detector de beacons
- Anunciador BLE com perfis configuráveis

### Sistema
- Configurações persistentes em NVS
- Interface TFT ILI9341 + touch XPT2046
- Botões físicos
- Monitor de uptime e memória
- Diagnósticos pela Serial

## Segurança

O firmware não implementa deauth attack, evil portal, beacon spam, BLE spam, captura de credenciais ou quebra de senhas. Recursos de rádio são destinados a diagnóstico, observação passiva e testes em redes e dispositivos próprios ou autorizados.

## Hardware

ESP32 Dual Core 240 MHz, 520 KB SRAM, TFT ILI9341 240x320 SPI, XPT2046, WiFi 802.11 b/g/n, BLE e flash de 4 MB.

## Licença

MIT License.
