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
    for (int row = 0; row < TOTAL_ROWS; row++) {
        html += "<tr>";
        bool isBottomRow = (row % 2 == 1);
        for (int col = 0; col < LEDS_PER_ROW; col++) {
            int actualCol = isBottomRow ? (LEDS_PER_ROW - 1 - col) : col;
            int ledIndex = row * LEDS_PER_ROW + actualCol;
            String color = ledStates[ledIndex] ? "#ffff00" : "#cccccc";
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
      if (request->hasParam("ssid", true)) ssid = request->getParam("ssid", true)->value();
      if (request->hasParam("pass", true)) pass = request->getParam("pass", true)->value();

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

        // Reboot
        server.on("/reboot", HTTP_GET, [&](AsyncWebServerRequest *request)
                  {
      request->send(200, "text/plain", "Neustart...");
      delay(300);
      ESP.restart(); });

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
      if (request->hasParam("ssid", true)) ssid = request->getParam("ssid", true)->value();
      if (request->hasParam("pass", true)) pass = request->getParam("pass", true)->value();

      prefs.begin("settings", false);
      prefs.putString("ntpServer", ntpServer);
      prefs.putLong("gmtOffset", gmtOffset_sec);
      prefs.putInt("daylightOffset", daylightOffset_sec);
      prefs.putInt("brightness", brightness);
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
        for (int i = 0; i < TOTAL_LEDS; i++) {
            if (i > 0) json += ",";
            json += ledStates[i] ? "1" : "0";
        }
        json += "]";
        request->send(200, "application/json", json);
    });

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
