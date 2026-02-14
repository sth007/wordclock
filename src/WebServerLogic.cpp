#include "WebServerLogic.h"

#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <vector>
#include <cstring>

#include "matrix.h"
#include "letters.h"
#include "config.h"
#include "timewords.h"

extern int brightness;
extern String ntpServer;
extern long gmtOffset_sec;
extern int daylightOffset_sec;
extern bool testMode;
extern bool firstLEDMode;
extern String ssid;
extern String pass;
extern int currentHour;
extern int currentMinute;
extern unsigned long diagLastLoopMs;
extern unsigned long diagLoopCounter;
extern unsigned long diagLastNtpSyncMs;
extern unsigned long diagLastWifiDisconnectMs;
extern int diagLastWifiDisconnectReason;
extern unsigned long diagLastWifiReconnectAttemptMs;
extern String diagLastWifiEvent;

// Extern für Funktionen aus main.cpp
extern void updateTime();

extern Adafruit_NeoPixel strip;

// Hilfsfunktion für Stundenwort
String getHourWord(int h) {
    h = h % 12;
    if (h == 0) h = 12;
    switch (h) {
        case 1: return "EINS";
        case 2: return "ZWEI";
        case 3: return "DREI";
        case 4: return "VIER";
        case 5: return "FUENF";
        case 6: return "SECHS";
        case 7: return "SIEBEN";
        case 8: return "ACHT";
        case 9: return "NEUN";
        case 10: return "ZEHN";
        case 11: return "ELF";
        case 12: return "ZWOELF";
        default: return "";
    }
}

// Funktion zur Generierung der Zeit in Wörtern
String getTimeWords(int hour, int minute) {
    String words = "ES IST";

    if (minute < 5) {
        words += " " + getHourWord(hour) + " UHR";
        return words;
    }

    if (minute < 10) {
        words += " FUENF NACH " + getHourWord(hour);
    }
    else if (minute < 15) {
        words += " ZEHN NACH " + getHourWord(hour);
    }
    else if (minute < 20) {
        words += " VIERTEL NACH " + getHourWord(hour);
    }
    else if (minute < 25) {
        words += " ZWANZIG NACH " + getHourWord(hour);
    }
    else if (minute < 30) {
        words += " FUENF VOR HALB " + getHourWord(hour + 1);
    }
    else if (minute < 35) {
        words += " HALB " + getHourWord(hour + 1);
    }
    else if (minute < 40) {
        words += " FUENF NACH HALB " + getHourWord(hour + 1);
    }
    else if (minute < 45) {
        words += " ZWANZIG VOR " + getHourWord(hour + 1);
    }
    else if (minute < 50) {
        words += " VIERTEL VOR " + getHourWord(hour + 1);
    }
    else if (minute < 55) {
        words += " ZEHN VOR " + getHourWord(hour + 1);
    }
    else {
        words += " FUENF VOR " + getHourWord(hour + 1);
    }
    return words;
}
// Funktion zur Generierung der LED-Matrix
String generateLEDMatrix() {
    String html = "<table style='border-collapse: collapse;'>";
    for (int row = 0; row < WORD_ROWS; row++) {
        html += "<tr>";
        for (int col = 0; col < LEDS_PER_ROW; col++) {
            int ledIndex = mapLogicalToPhysical(row, col);
            String color = (ledIndex >= 0 && ledStates[ledIndex]) ? "#ffff00" : "#cccccc";
            html += "<td style='width:20px; height:20px; background-color:" + color + "; border:1px solid #999;'></td>";
        }
        html += "</tr>";
    }
    html += "</table>";
    return html;
}

// Template-Processor für HTML-Platzhalter
String processor(const String& var) {
  if (var == "currentTime") {
    return String(currentHour) + ":" + (currentMinute < 10 ? "0" : "") + String(currentMinute);
  } else if (var == "brightness") {
    return String(brightness);
  } else if (var == "ntpServer") {
    return ntpServer;
  } else if (var == "gmtOffset") {
    return String(gmtOffset_sec / 3600);
  } else if (var == "daylightOffset") {
    return String(daylightOffset_sec / 3600);
  } else if (var == "testMode") {
    return ::testMode ? "Aktiv (alle LEDs)" : "Inaktiv";
  } else if (var == "firstLEDMode") {
    return ::firstLEDMode ? "Aktiv" : "Inaktiv";
  } else if (var == "ipAddress") {
    return WiFi.localIP().toString();
  } else if (var == "testToggle") {
    return ::testMode ? "deaktivieren" : "aktivieren";
  } else if (var == "firstLEDToggle") {
    return ::firstLEDMode ? "deaktivieren" : "aktivieren";
  } else if (var == "ssid") {
    return ssid;
  } else if (var == "ledPin") {
    return String(LED_PIN);
  } else if (var == "ledsPerRow") {
    return String(LEDS_PER_ROW);
  } else if (var == "wordRows") {
    return String(WORD_ROWS);
  } else if (var == "ledOrigin") {
    return String(getLedOrigin());
  } else if (var == "ledOriginName") {
    return String(getLedOriginName());
  } else if (var == "ledMatrix") {
    return generateLEDMatrix();
  } else if (var == "timeWords") {
    return getTimeWords(currentHour, currentMinute);
  }
  return String();
}
extern bool testMode;

namespace
{
    bool fsReady = false;
    bool fsInitDone = false;
    File uploadFile;
    String uploadPath;
    bool uploadFailed = false;
    bool wordMapLoaded = false;
    const char *WORDMAP_NS = "wordmap";

    bool isAllowedBackgroundFile(const String &filename)
    {
        String lower = filename;
        lower.toLowerCase();
        return lower.endsWith(".jpg") || lower.endsWith(".jpeg") || lower.endsWith(".png");
    }

    String getBackgroundPath()
    {
        if (LittleFS.exists("/www/background.jpg"))
            return "/www/background.jpg";
        if (LittleFS.exists("/www/background.jpeg"))
            return "/www/background.jpeg";
        if (LittleFS.exists("/www/background.png"))
            return "/www/background.png";
        return "";
    }

    String getMimeTypeForPath(const String &path)
    {
        if (path.endsWith(".png"))
            return "image/png";
        if (path.endsWith(".jpeg") || path.endsWith(".jpg"))
            return "image/jpeg";
        return "application/octet-stream";
    }

    bool parseCsvLeds(const String &csv, std::vector<int> &out)
    {
        out.clear();
        int start = 0;
        while (start < csv.length()) {
            int comma = csv.indexOf(',', start);
            String token = (comma < 0) ? csv.substring(start) : csv.substring(start, comma);
            token.trim();
            if (token.length() > 0) {
                int value = token.toInt();
                if (value < 0 || value >= LEDS_PER_ROW) {
                    return false;
                }
                bool duplicate = false;
                for (int v : out) {
                    if (v == value) {
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate) {
                    out.push_back(value);
                }
            }
            if (comma < 0) {
                break;
            }
            start = comma + 1;
        }
        return !out.empty();
    }

    bool parseCsvWords(const String &csv, std::vector<String> &out)
    {
        out.clear();
        int start = 0;
        while (start <= csv.length()) {
            int comma = csv.indexOf(',', start);
            String token = (comma >= 0) ? csv.substring(start, comma) : csv.substring(start);
            token.trim();
            if (!token.isEmpty()) {
                out.push_back(token);
            }
            if (comma < 0) {
                break;
            }
            start = comma + 1;
        }
        return true;
    }

    bool isKnownWord(const String &word)
    {
        int count = getWordCount();
        for (int i = 0; i < count; i++) {
            const char *name = getWordNameAt(i);
            if (name && word.equals(name)) {
                return true;
            }
        }
        return false;
    }

    const char* wifiModeName(wifi_mode_t mode)
    {
        switch (mode) {
        case WIFI_STA: return "STA";
        case WIFI_AP: return "AP";
        case WIFI_AP_STA: return "AP_STA";
        case WIFI_MODE_NULL:
        default:
            return "OFF";
        }
    }

    const char* wifiStatusName(wl_status_t status)
    {
        switch (status) {
        case WL_CONNECTED: return "CONNECTED";
        case WL_DISCONNECTED: return "DISCONNECTED";
        case WL_CONNECTION_LOST: return "CONNECTION_LOST";
        case WL_NO_SSID_AVAIL: return "NO_SSID";
        case WL_CONNECT_FAILED: return "CONNECT_FAILED";
        case WL_IDLE_STATUS: return "IDLE";
        case WL_SCAN_COMPLETED: return "SCAN_COMPLETED";
        default:
            return "UNKNOWN";
        }
    }

    bool saveWordMappingsToNvs()
    {
        Preferences prefs;
        if (!prefs.begin(WORDMAP_NS, false)) {
            return false;
        }

        int count = getWordCount();
        for (int i = 0; i < count; i++) {
            const char *name = getWordNameAt(i);
            String csv = "";
            int ledCount = getWordLedCountAt(i);
            for (int j = 0; j < ledCount; j++) {
                if (j > 0) {
                    csv += ",";
                }
                csv += String(getWordLedAt(i, j));
            }
            prefs.putString(name, csv);
        }
        prefs.end();
        return true;
    }

    void loadWordMappingsFromNvs()
    {
        if (wordMapLoaded) {
            return;
        }
        wordMapLoaded = true;

        Preferences prefs;
        if (!prefs.begin(WORDMAP_NS, true)) {
            return;
        }

        int count = getWordCount();
        for (int i = 0; i < count; i++) {
            const char *name = getWordNameAt(i);
            String csv = prefs.getString(name, "");
            if (csv.isEmpty()) {
                continue;
            }
            csv.trim();
            std::vector<int> leds;
            if (!parseCsvLeds(csv, leds)) {
                continue;
            }
            setWordLedsByName(name, leds.data(), leds.size());
        }
        prefs.end();
    }

    String getWordMappingsJson()
    {
        String json = "{";
        int count = getWordCount();
        for (int i = 0; i < count; i++) {
            if (i > 0) {
                json += ",";
            }
            json += "\"";
            json += getWordNameAt(i);
            json += "\":[";
            int ledCount = getWordLedCountAt(i);
            for (int j = 0; j < ledCount; j++) {
                if (j > 0) {
                    json += ",";
                }
                json += String(getWordLedAt(i, j));
            }
            json += "]";
        }
        json += "}";
        return json;
    }

    void ensureFs()
    {
        if (fsInitDone)
            return;
        fsInitDone = true;
        fsReady = LittleFS.begin(true); // format on fail
    }

    void sendIndexHtml(AsyncWebServerRequest *request)
    {
        ensureFs();
        if (!fsReady)
        {
            request->send(500, "text/plain", "LittleFS init fehlgeschlagen");
            return;
        }

        const char *path = "/www/index.html";
        if (LittleFS.exists(path))
            request->send(LittleFS, path, "text/html");
        else
            request->send(404, "text/plain", "Fehlt: /www/index.html (uploadfs vergessen?)");
    }
}

namespace WebServerLogic
{
    bool isFsReady()
    {
        ensureFs();
        return fsReady;
    }

    void begin(AsyncWebServer &server, Preferences &prefs)
    {
        loadWordMappingsFromNvs();

        // Root: Im AP-Modus Setup-HTML, sonst Status
        server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      if (WiFi.getMode() == WIFI_AP)
        sendIndexHtml(request);
      else {
        ensureFs();
        if (!fsReady) {
          request->send(500, "text/plain", "LittleFS init fehlgeschlagen");
        } else {
          File file = LittleFS.open("/www/status.html", "r");
          if (file) {
            String html = file.readString();
            file.close();
            // Ersetze alle Platzhalter
            html.replace("{{currentTime}}", processor("currentTime"));
            html.replace("{{brightness}}", processor("brightness"));
            html.replace("{{ntpServer}}", processor("ntpServer"));
            html.replace("{{gmtOffset}}", processor("gmtOffset"));
            html.replace("{{daylightOffset}}", processor("daylightOffset"));
            html.replace("{{testMode}}", processor("testMode"));
            html.replace("{{firstLEDMode}}", processor("firstLEDMode"));
            html.replace("{{ipAddress}}", processor("ipAddress"));
            html.replace("{{testToggle}}", processor("testToggle"));
            html.replace("{{firstLEDToggle}}", processor("firstLEDToggle"));
            html.replace("{{ledMatrix}}", processor("ledMatrix"));
            html.replace("{{timeWords}}", processor("timeWords"));
            request->send(200, "text/html", html);
          } else {
            request->send(404, "text/plain", "Datei nicht gefunden");
          }
        }
      } });

        // WLAN speichern
        server.on("/save", HTTP_POST, [&](AsyncWebServerRequest *request)
                  {
      String ssid, pass;
      if (request->hasParam("ssid", true)) {
        String newSsid = request->getParam("ssid", true)->value();
        newSsid.trim();
        if (!newSsid.isEmpty()) {
          ssid = newSsid;
        }
      }
      if (request->hasParam("pass", true)) {
        String newPass = request->getParam("pass", true)->value();
        if (!newPass.isEmpty()) {
          pass = newPass;
        }
      }

      prefs.begin("wifi", false);
      prefs.putString("ssid", ssid);
      prefs.putString("pass", pass);
      prefs.end();

      request->send(200, "text/plain", "Gespeichert. Neustart...");
      delay(300);
      ESP.restart(); });

        // WLAN löschen
        server.on("/reset", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      prefs.begin("wifi", false);
      prefs.clear();
      prefs.end();

      request->send(200, "text/plain", "WLAN-Daten geloescht. Neustart...");
      delay(300);
      ESP.restart(); });

        // Helligkeit setzen
        server.on("/brightness", HTTP_POST, [&](AsyncWebServerRequest *request)
                  {
      if (request->hasParam("brightness", true)) {
        int val = request->getParam("brightness", true)->value().toInt();
        if (val >= 1 && val <= 10) {
          ::brightness = val;
          prefs.begin("settings", false);
          prefs.putInt("brightness", ::brightness);
          prefs.end();
          request->send(200, "text/plain", "Helligkeit gesetzt: " + String(::brightness));
        } else {
          request->send(400, "text/plain", "Ungueltiger Wert (1-10)");
        }
      } else {
        request->send(400, "text/plain", "Parameter 'brightness' fehlt");
      } });

        // OTA Status
        server.on("/ota", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      request->send(200, "text/plain", "OTA bereit. Verbinde mit IDE."); });

        // LED-Test umschalten
        server.on("/test", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      ::testMode = !::testMode;
      if (::testMode) ::firstLEDMode = false; // Deaktiviere anderen Test
      prefs.begin("settings", false);
      prefs.putBool("testMode", ::testMode);
      prefs.putBool("firstLEDMode", ::firstLEDMode);
      prefs.end();
      if (::testMode) {
        showAllWords();
      } else {
        updateTime();
        showTime(currentHour, currentMinute);
      }
      request->redirect("/"); });

        // Erste LED-Test umschalten
        server.on("/testfirst", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      ::firstLEDMode = !::firstLEDMode;
      if (::firstLEDMode) ::testMode = false; // Deaktiviere anderen Test
      prefs.begin("settings", false);
      prefs.putBool("firstLEDMode", ::firstLEDMode);
      prefs.putBool("testMode", ::testMode);
      prefs.end();
      if (::firstLEDMode) {
        showFirstLED();
      } else {
        updateTime();
        showTime(currentHour, currentMinute);
      }
      request->redirect("/"); });

        // Alle Wörter gleichzeitig Test
        server.on("/testallatonce", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      showAllWordsAtOnce();
      request->send(200, "text/plain", "Alle Wörter leuchten!");
      request->redirect("/"); });

        // Reboot
        server.on("/reboot", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      request->send(200, "text/plain", "Neustart...");
      delay(300);
      ESP.restart(); });

        // Normale Uhrzeitanzeige aktivieren (beide Testmodi aus)
        server.on("/normaltime", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      ::testMode = false;
      ::firstLEDMode = false;
      prefs.begin("settings", false);
      prefs.putBool("testMode", ::testMode);
      prefs.putBool("firstLEDMode", ::firstLEDMode);
      prefs.end();
      updateTime();
      showTime(currentHour, currentMinute);
      request->send(200, "text/plain", "Normale Uhrzeitanzeige aktiviert"); });

        // Konfiguration
        server.on("/config", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      ensureFs();
      if (!fsReady) {
        request->send(500, "text/plain", "LittleFS init fehlgeschlagen");
      } else {
        File file = LittleFS.open("/www/config.html", "r");
        if (file) {
          String html = file.readString();
          file.close();
          // Ersetze Platzhalter
          html.replace("{{currentTime}}", processor("currentTime"));
          html.replace("{{ntpServer}}", processor("ntpServer"));
          html.replace("{{gmtOffset}}", processor("gmtOffset"));
          html.replace("{{daylightOffset}}", processor("daylightOffset"));
          html.replace("{{brightness}}", processor("brightness"));
          html.replace("{{ssid}}", processor("ssid"));
          html.replace("{{timeWords}}", processor("timeWords"));
          html.replace("{{testMode}}", processor("testMode"));
          html.replace("{{firstLEDMode}}", processor("firstLEDMode"));
          html.replace("{{ledOrigin}}", processor("ledOrigin"));
          request->send(200, "text/html", html);
        } else {
          request->send(404, "text/plain", "Datei nicht gefunden");
        }
      } });

        server.on("/config", HTTP_POST, [&](AsyncWebServerRequest *request)
                  {

      if (request->hasParam("ntpServer", true)) ntpServer = request->getParam("ntpServer", true)->value();
      if (request->hasParam("gmtOffset", true)) gmtOffset_sec = request->getParam("gmtOffset", true)->value().toInt() * 3600;
      if (request->hasParam("daylightOffset", true)) daylightOffset_sec = request->getParam("daylightOffset", true)->value().toInt() * 3600;
      if (request->hasParam("brightness", true)) brightness = request->getParam("brightness", true)->value().toInt();
      if (request->hasParam("ledOrigin", true)) setLedOrigin(request->getParam("ledOrigin", true)->value().toInt());
      if (request->hasParam("ssid", true)) ssid = request->getParam("ssid", true)->value();
      if (request->hasParam("pass", true)) pass = request->getParam("pass", true)->value();

      prefs.begin("settings", false);
      prefs.putString("ntpServer", ntpServer);
      prefs.putLong("gmtOffset", gmtOffset_sec);
      prefs.putInt("daylightOffset", daylightOffset_sec);
      prefs.putInt("brightness", brightness);
      prefs.putInt("ledOrigin", getLedOrigin());
      prefs.end();

      prefs.begin("wifi", false);
      prefs.putString("ssid", ssid);
      prefs.putString("pass", pass);
      prefs.end();

      request->send(200, "text/plain", "Gespeichert. Neustart...");
      delay(300);
      ESP.restart(); });

    // API-Endpunkt für LED-Zustände
    server.on("/api/leds", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "[";
        int outIndex = 0;
        for (int row = 0; row < WORD_ROWS; row++) {
            for (int col = 0; col < LEDS_PER_ROW; col++) {
                if (outIndex++ > 0) json += ",";
                int ledIndex = mapLogicalToPhysical(row, col);
                json += (ledIndex >= 0 && ledStates[ledIndex]) ? "1" : "0";
            }
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    server.on("/api/words", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", getWordMappingsJson());
    });

    server.on("/api/info", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"currentTime\":\"" + processor("currentTime") + "\",";
        json += "\"timeWords\":\"" + processor("timeWords") + "\",";
        json += "\"testMode\":" + String(::testMode ? "true" : "false") + ",";
        json += "\"firstLEDMode\":" + String(::firstLEDMode ? "true" : "false");
        json += "}";
        request->send(200, "application/json", json);
    });

    server.on("/api/health", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"uptimeMs\":" + String(millis()) + ",";
        json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"loopCounter\":" + String(diagLoopCounter) + ",";
        json += "\"lastLoopMs\":" + String(diagLastLoopMs) + ",";
        json += "\"lastNtpSyncMs\":" + String(diagLastNtpSyncMs) + ",";
        json += "\"lastWifiDisconnectMs\":" + String(diagLastWifiDisconnectMs) + ",";
        json += "\"lastWifiDisconnectReason\":" + String(diagLastWifiDisconnectReason) + ",";
        json += "\"lastWifiReconnectAttemptMs\":" + String(diagLastWifiReconnectAttemptMs) + ",";
        json += "\"lastWifiEvent\":\"" + diagLastWifiEvent + "\",";
        json += "\"wifiMode\":\"" + String(wifiModeName(WiFi.getMode())) + "\",";
        json += "\"wifiStatus\":\"" + String(wifiStatusName(WiFi.status())) + "\",";
        json += "\"rssi\":" + String((WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0) + ",";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
        json += "}";
        request->send(200, "application/json", json);
    });

    server.on("/api/word", HTTP_POST, [](AsyncWebServerRequest *request) {
        ensureFs();
        if (!fsReady) {
            request->send(500, "text/plain", "LittleFS init fehlgeschlagen");
            return;
        }

        if (!request->hasParam("word", true) || !request->hasParam("leds", true)) {
            request->send(400, "text/plain", "Parameter 'word' und 'leds' erforderlich");
            return;
        }

        String word = request->getParam("word", true)->value();
        String ledsCsv = request->getParam("leds", true)->value();
        word.trim();
        ledsCsv.trim();
        if (word.isEmpty() || ledsCsv.isEmpty()) {
            request->send(400, "text/plain", "Leere Parameter nicht erlaubt");
            return;
        }

        std::vector<int> leds;
        if (!parseCsvLeds(ledsCsv, leds)) {
            request->send(400, "text/plain", "Ungueltige LED-Liste");
            return;
        }

        if (!setWordLedsByName(word.c_str(), leds.data(), leds.size())) {
            request->send(400, "text/plain", "Unbekanntes Wort oder ungueltige Daten");
            return;
        }

        if (!saveWordMappingsToNvs()) {
            request->send(500, "text/plain", "Speichern fehlgeschlagen");
            return;
        }

        request->send(200, "text/plain", "Gespeichert");
    });

    server.on("/api/preview-words", HTTP_POST, [&](AsyncWebServerRequest *request) {
        String wordsCsv = "";
        if (request->hasParam("words", true)) {
            wordsCsv = request->getParam("words", true)->value();
        }
        wordsCsv.trim();

        std::vector<String> words;
        if (!parseCsvWords(wordsCsv, words)) {
            request->send(400, "text/plain", "Ungueltige Wortliste");
            return;
        }

        for (const String &word : words) {
            if (!isKnownWord(word)) {
                request->send(400, "text/plain", "Unbekanntes Wort: " + word);
                return;
            }
        }

        ::testMode = false;
        ::firstLEDMode = false;
        prefs.begin("settings", false);
        prefs.putBool("testMode", ::testMode);
        prefs.putBool("firstLEDMode", ::firstLEDMode);
        prefs.end();

        clearAll();
        uint32_t color = dimColor(20, 20, 20);
        for (const String &word : words) {
            lightWord(word.c_str(), color);
        }

        request->send(200, "text/plain", "Vorschau aktualisiert");
    });

    server.on("/api/background", HTTP_GET, [](AsyncWebServerRequest *request) {
        ensureFs();
        if (!fsReady) {
            request->send(500, "text/plain", "LittleFS init fehlgeschlagen");
            return;
        }
        String path = getBackgroundPath();
        if (path.isEmpty()) {
            request->send(404, "text/plain", "Kein Hintergrundbild vorhanden");
            return;
        }
        request->send(LittleFS, path, getMimeTypeForPath(path));
    });

    server.on("/upload-bg", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            ensureFs();
            if (!fsReady) {
                request->send(500, "text/plain", "LittleFS init fehlgeschlagen");
                return;
            }
            if (uploadFailed) {
                request->send(400, "text/plain", "Upload fehlgeschlagen (nur JPG/PNG erlaubt)");
                uploadFailed = false;
                uploadPath = "";
                return;
            }
            request->send(200, "text/plain", "Hintergrundbild gespeichert");
        },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            (void)request;

            if (index == 0) {
                uploadFailed = false;
                uploadPath = "";

                if (!isAllowedBackgroundFile(filename)) {
                    uploadFailed = true;
                    return;
                }

                String lower = filename;
                lower.toLowerCase();
                uploadPath = lower.endsWith(".png") ? "/www/background.png" : "/www/background.jpg";

                LittleFS.remove("/www/background.jpg");
                LittleFS.remove("/www/background.jpeg");
                LittleFS.remove("/www/background.png");

                uploadFile = LittleFS.open(uploadPath, "w");
                if (!uploadFile) {
                    uploadFailed = true;
                    return;
                }
            }

            if (uploadFailed) {
                if (final && !uploadPath.isEmpty()) {
                    LittleFS.remove(uploadPath);
                    uploadPath = "";
                }
                return;
            }

            if (uploadFile && len > 0) {
                if (uploadFile.write(data, len) != len) {
                    uploadFailed = true;
                }
            }

            if (final) {
                if (uploadFile) {
                    uploadFile.close();
                }
                if (uploadFailed && !uploadPath.isEmpty()) {
                    LittleFS.remove(uploadPath);
                }
                uploadPath = "";
            }
        }
    );

    // Statische Dateien aus LittleFS servieren
    server.onNotFound([](AsyncWebServerRequest *request) {
        String path = request->url();
        ensureFs();
        if (!fsReady) {
            request->send(500, "text/plain", "LittleFS init fehlgeschlagen");
            return;
        }
        String fsPath = "/www" + path;
        if (LittleFS.exists(fsPath)) {
            // MIME-Type bestimmen
            String mime = "text/plain";
            if (path.endsWith(".html")) mime = "text/html";
            else if (path.endsWith(".css")) mime = "text/css";
            else if (path.endsWith(".js")) mime = "application/javascript";
            else if (path.endsWith(".jpeg") || path.endsWith(".jpg")) mime = "image/jpeg";
            else if (path.endsWith(".png")) mime = "image/png";
            request->send(LittleFS, fsPath, mime);
        } else {
            request->send(404, "text/plain", "Datei nicht gefunden");
        }
    });

        server.begin();
    }
}
