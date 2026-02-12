# Copilot / AI-Agent Hinweise — Wordclock

Kurz und praktisch: diese Datei hilft AI-Codern, sofort produktiv zu werden.

## Kurzübersicht (Big picture)
- **Ziel**: ESP32-C3 basierte Wordclock mit WiFi-Setup-AP, Web-UI und NTP-Zeitsynchronisation.
- **Boot-Flow**: `setup()` verbindet sich mit gespeichertem WLAN (Preferences) oder startet AP-Modus. `loop()` aktualisiert Zeit alle 60s und zeigt Wort-Zeit auf LED-Matrix (23×20 NeoPixels).
- **Web-Architektur**: AsyncWebServer registriert Routen in `WebServerLogic::begin()`. Statische Assets (HTML/CSS/JS) liegen in `data/www/` → LittleFS.
- **LED-Display**: 460 Adafruit NeoPixel in 10 Wort-Reihen à 2 Zeilen. Wortlayout ist fest in `letters.cpp` definiert. `showTime()` setzt aktive LEDs entsprechend Stunde/Minute.
- **Persistierung**: Zwei Preferences-Namespaces — `"wifi"` für Credentials (readonly load, readwrite save), `"settings"` für NTP/Helligkeit/Test-Flags.

## Wichtige Dateien (schnellreferenz)
| Datei | Rolle |
|-------|-------|
| `src/main.cpp` | Boot, WLAN-Connect, OTA-Handler, Loop (Zeit-Update, LED-Anzeige) |
| `src/WebServerLogic.cpp/.h` | HTTP-Routen: `/`, `/save`, `/reset`, `/config`, `/brightness`, `/test`, `/testfirst`, `/reboot`, `/api/leds` |
| `src/matrix.cpp/.h` | LED-Matrix-Konstanten (LEDS_PER_ROW=23, TOTAL_ROWS=20), Hilfsfunktionen |
| `src/letters.cpp` | Wort-Definitions-Arrays (WORD_ES, WORD_ZEHN, etc.) als Index-Listen |
| `include/config.h` | Zentrale Konstanten: LED_PIN=8, NTP_SERVER, AP_SSID="ESP32-Setup", Helligkeitsregeln |
| `data/www/{index,config,status}.html` | Web-UI mit Template-Platzhaltern (`{{currentTime}}`, `{{brightness}}`, etc.) |

## Projekt-spezifische Patterns & gotchas

### Persistierung (Preferences)
- **WiFi-Namespace** (`"wifi"`): SSID/Pass — `main.cpp` readonly load im `setup()`, `WebServerLogic` readwrite save+restart.
- **Settings-Namespace** (`"settings"`): brightness, ntpServer, gmtOffset, daylightOffset, testMode, firstLEDMode.
- **Wichtig**: Immer `prefs.end()` nach `begin()` aufrufen, auch bei Exceptions (oder Guard nutzen).

### LittleFS & Web-Assets
- `LittleFS.begin(true)` formatiert Filesystem bei Fehler automatisch.
- Funktionen `sendIndexHtml()` und `ensureFs()` in `WebServerLogic.cpp` prüfen Existenz; Return 500/404 bei Fehler.
- **Neue HTML-Dateien hinzufügen**: In `data/www/` platzieren, mit `pio run -t uploadfs` hochladen.

### LED-Anzeige & Test-Modi
- `ledStates[TOTAL_LEDS]` ist globales Boolean-Array (extern in `letters.h`).
- `showTime(hour, minute)` setzt LEDs für Deutsche Wortuhr (z.B. "ES IST ZEHN VOR EINS" um 12:50).
- **Test-Modi**: `/test` schaltet `testMode` (zeigt alle Wörter), `/testfirst` zeigt nur LED 0. Speichern in Preferences + Reload.
- **Nachtmodus**: Automatisch 22:00–05:59, setzt `brightness = NIGHT_BRIGHTNESS` (=2, sonst DEFAULT=10).

### Webserver & Asynchronität
- `ESPAsyncWebServer` mit Lambda-Captures (`[&]`) für Zugriff auf globale Variablen (ssid, brightness, etc.).
- Handler wie `/save`, `/reset`, `/reboot` führen `ESP.restart()` nach `delay(300)` aus — **intentional blocking**.
- `processor(String var)` ersetzt HTML-Template-Tags (`{{currentTime}}`, `{{brightness}}`, etc.).

### API-Endpunkte
- `GET /api/leds` — JSON-Array mit LED-Status (0/1) für Live-Anzeigen.
- `POST /save` — speichert SSID/Pass, reboot.
- `POST /config` — speichert NTP, GMT-Offset, Helligkeit, speichert Credentials auch in "wifi".
- `GET /` — AP-Modus → `index.html`, Station-Modus → `status.html` mit Live-Status.

## Build / Upload / Debug
```bash
# Firmware kompilieren
pio run --environment esp32-c3

# Firmware hochladen (OTA über WiFi, default in platformio.ini)
pio run -t upload --environment esp32-c3

# LittleFS-Dateien hochladen (notwendig nach data/www-Änderungen)
pio run -t uploadfs --environment esp32-c3

# OTA-Firmware-Upload (wenn mit WLAN verbunden)
pio run -t uploadfsota --environment esp32-c3

# Serieller Monitor (Debug-Output, Serial.println)
pio device monitor --environment esp32-c3

# USB-Upload (falls OTA fehlschlägt) — ändern in platformio.ini:
# upload_protocol = esptool
# upload_port = /dev/cu.usbmodemXXXX
```

## Rules für sichere AI-Edits
- **Web-Routen ändern**: Erhalte die HTTP-Methode (GET/POST) und Parameter-Namen. Wenn `/save` sich ändert, aktualisiere `data/www/index.html` Form-Action.
- **Preferences**: Nutzung von `begin()/end()` ist zwingend. Read-only (`true`) wo möglich, readwrite (`false`) nur beim Speichern.
- **LED-Matrix**: Keine hardcodeten Indizes — verwende WORD_*-Konstanten aus `letters.h` und `matrix.h`.
- **Neue globale extern-Variablen**: Immer in passender Header-Datei deklarieren, nicht inline in CPP.
- **Error-Handling**: LittleFS-Fehler mit aussagekräftigen HTTP-Response (404/500) + Serial.println() für Debugging.

## Integration & Dependencies
- **PlatformIO**: `espressif32` Platform, Board `esp32-c3-devkitm-1`.
- **Libs** (lib_deps in platformio.ini): `AsyncTCP-esphome`, `ESPAsyncWebServer-esphome`, `ESPmDNS`, `Adafruit NeoPixel`.
- **LittleFS**: `board_build.filesystem = littlefs` in platformio.ini aktiviert Filesystem.
- **OTA**: Basis-Setup in `src/ota.cpp`, aktiviert wenn WLAN verbunden (`setupOTA(HOSTNAME)`).

## Quick-Beispiele
```bash
# WLAN im AP-Modus speichern (curl)
curl -X POST -F "ssid=MeinNetz" -F "pass=secret" http://192.168.4.1/save

# Nach WLAN-Verbindung über mDNS
curl http://wordclock.local/status

# Helligkeit auf 5 setzen
curl -X POST -F "brightness=5" http://wordclock.local/brightness

# Test-Modus an
curl http://wordclock.local/test
```

## Wichtige Globals (extern)
- `ssid`, `pass` — WLAN-Credentials aus Preferences.
- `brightness` — 1–10, Nachtmodus setzt automatisch auf 2.
- `currentHour`, `currentMinute` — von NTP/updateTime() aktualisiert.
- `ntpServer`, `gmtOffset_sec`, `daylightOffset_sec` — Zeitsync-Config.
- `testMode`, `firstLEDMode` — Boolean-Flags für Test-Anzeigen.
- `ledStates[TOTAL_LEDS]` — Boolean-Array, aktueller LED-Status.
- `strip` (Adafruit_NeoPixel) — Hardware-Interface zu NeoPixels.
