# WordClock ESP32-C3

Eine digitale Wordclock basierend auf ESP32-C3 mit WiFi, Web-UI, NTP-Synchronisation und LED-Anzeige.

## Features

- **Zeitanzeige**: Deutsche Wortuhr mit 460 LEDs (Adafruit NeoPixel).
- **WiFi-Verbindung**: Automatische Verbindung zu WLAN, Fallback auf Access Point.
- **Web-UI**: Konfiguration über Browser (NTP, Helligkeit, WLAN-Einstellungen).
- **NTP-Synchronisation**: Automatische Zeitsynchronisation.
- **LED-Tests**: Verschiedene Testmodi für LEDs (alle anzeigen, erste LED, etc.).
- **OTA-Updates**: Drahtlose Firmware- und Dateisystem-Updates.
- **Night Mode**: Automatische Helligkeitsanpassung.
- **Responsive Design**: Mobile-freundliche Weboberfläche.

## Installation

### Voraussetzungen

- PlatformIO (für Build und Upload).
- ESP32-C3 DevKitM-1 Board.
- USB-Kabel für initialen Upload.

### Schritte

1. **Repository klonen**:
   ```bash
   git clone https://github.com/yourusername/wordclock.git
   cd wordclock
   ```

2. **Abhängigkeiten installieren**:
   PlatformIO installiert automatisch die benötigten Libraries.

3. **Board anschließen**:
   Verbinde das ESP32-C3 über USB.

4. **Seriellen Port finden**:
   ```bash
   pio device list
   ```
   Suche nach `/dev/cu.usbmodemXXXX` (z.B. `/dev/cu.usbmodem14101`).

5. **PlatformIO konfigurieren** (für USB-Upload):
   In `platformio.ini`:
   ```ini
   upload_protocol = esptool
   upload_port = /dev/cu.usbmodem14101
   monitor_port = /dev/cu.usbmodem14101
   ```

6. **Firmware bauen und hochladen**:
   ```bash
   pio run --environment esp32-c3
   pio run -t upload --environment esp32-c3
   ```

7. **Dateisystem hochladen** (HTML-Dateien):
   ```bash
   pio run -t uploadfs --environment esp32-c3
   ```

## Konfiguration

Nach dem ersten Start startet die Wordclock im Access Point-Modus (SSID: wordclock, Passwort: leer).

1. **WLAN verbinden**:
   - Verbinde dich mit dem WLAN "wordclock".
   - Öffne `http://192.168.4.1` im Browser.

2. **WLAN-Einstellungen konfigurieren**:
   - Gib SSID und Passwort deines WLANs ein.
   - Speichere und starte neu.

3. **Weitere Einstellungen**:
   - NTP-Server, Zeitzone, Helligkeit über die Web-UI anpassen.
   - Zugriff über `http://wordclock.local` (nach WLAN-Verbindung).

## Update

### OTA-Update (empfohlen)

1. Stelle sicher, dass das Gerät mit WLAN verbunden ist.
2. Führe aus:
   ```bash
   pio run -t upload --environment esp32-c3
   ```
   Für Dateisystem-Updates:
   ```bash
   pio run -t uploadfsota --environment esp32-c3
   ```

### USB-Update

Falls OTA nicht funktioniert:
1. Schließe das Board über USB an.
2. Setze `upload_protocol = esptool` in `platformio.ini`.
3. Führe `pio run -t upload --environment esp32-c3` aus.

## Verwendung

- **Zeitanzeige**: Die Uhr zeigt die Zeit in Worten an.
- **Web-UI**: Öffne `http://wordclock.local` für Konfiguration und Tests.
- **LED-Tests**: Über die Web-UI verschiedene Testmodi aktivieren.
- **Monitor**: Serielle Ausgabe anzeigen mit `pio device monitor --environment esp32-c3`.

## Projektstruktur

- `src/`: Quellcode (main.cpp, WebServerLogic.cpp, etc.)
- `include/`: Header-Dateien (config.h, etc.)
- `data/www/`: HTML-Dateien für Web-UI
- `platformio.ini`: Build-Konfiguration

## Lizenz

[MIT License](LICENSE)

## Beiträge

Pull Requests sind willkommen! Bitte teste Änderungen gründlich.