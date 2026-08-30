# ESP32 WiFi Toolkit v9.0

Firmware portátil para diagnóstico WiFi/BLE e observação passiva usando ESP32, ILI9341 e XPT2046.

## O que está implementado

### WiFi
- Scanner de redes com seleção e ordenação por RSSI
- Detalhes de SSID, BSSID, canal, frequência, segurança e fabricante OUI
- Scanner de ocupação dos canais 2.4 GHz
- Visualização de nível WiFi por canal
- Medidor de sinal com histórico
- Detecção de SSIDs ocultos por varredura
- Otimizador de canal baseado em quantidade, RSSI e sobreposição
- Diagnóstico de gateway, DNS e HTTP 204
- Detector de portal cativo e redirecionamentos
- Monitor passivo de frames 802.11
- Detector passivo de deauthentication/desassociation

### Bluetooth LE
- Scanner com seleção de dispositivo
- Detector de iBeacon e Eddystone
- Monitor de proximidade por RSSI
- Inspetor GATT para dispositivos autorizados
- Leitura de características com permissão READ
- Anunciador BLE local com intervalo configurável

### Sistema
- Configurações persistentes em NVS
- Brilho do TFT
- Canal WiFi padrão
- Duração dos scans
- Scan de redes ocultas
- Preferência de ordenação por RSSI
- Calibração do XPT2046
- Reset para valores padrão
- Uptime, heap livre, heap mínimo, flash e MAC

## Estrutura

Os módulos são separados em arquivos `.h/.cpp` para evitar que a lógica de rádio, interface e armazenamento fique concentrada no sketch principal.

`tools/audit_sources.py` verifica a estrutura dos fontes e procura alguns erros conhecidos de integração BLE/802.11.

`tools/update_oui.py` gera uma tabela OUI maior usando o registro público da IEEE. A versão compacta do repositório serve como fallback para não inflar o firmware sem necessidade.

## Hardware

- ESP32
- TFT ILI9341 240x320
- Touch XPT2046
- WiFi 802.11 b/g/n
- BLE
- Flash 4 MB

### TFT

| Sinal | GPIO |
|---|---:|
| MOSI | 23 |
| MISO | 19 |
| SCLK | 18 |
| CS | 15 |
| DC | 2 |
| RST | 4 |
| LED | 27 |

### Touch

| Sinal | GPIO |
|---|---:|
| CLK | 14 |
| MOSI | 13 |
| MISO | 12 |
| CS | 5 |
| IRQ | 21 |

### Botões

| Função | GPIO |
|---|---:|
| PREV | 32 |
| NEXT | 33 |
| SELECT | 25 |
| BACK | 26 |

## Build

Arduino IDE pode ser usado com as bibliotecas:

- TFT_eSPI 2.5+
- XPT2046_Touchscreen 1.4+

Também existe `platformio.ini` para build reproduzível.

```bash
python tools/audit_sources.py
pio run -e esp32dev
```

O workflow em `.github/workflows/build.yml` executa essas verificações automaticamente no GitHub Actions.

## Uso responsável

O firmware foi mantido sem deauth attack, evil portal, beacon spam, BLE spam, captura de credenciais ou quebra de senhas. Recursos de rádio devem ser usados somente em redes e dispositivos próprios ou autorizados.

O detector de deauth é exclusivamente passivo. O analisador de senha trabalha offline e não tenta autenticar, adivinhar ou testar senhas em redes.

## Licença

MIT License.
