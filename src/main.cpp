#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <time.h>

#include <Adafruit_NeoPixel.h>
#include <ESPAsyncWebServer.h>

#include "WebServerLogic.h"
#include "ota.h"

#include "matrix.h"
#include "letters.h"
#include "timewords.h"
#include "config.h"

// ===== Hardware =====
AsyncWebServer server(80);
Preferences prefs;

// ===== WLAN =====
String ssid, pass;

// ===== LED Strip =====
Adafruit_NeoPixel strip(getTotalLEDs(), LED_PIN, NEO_GRB + NEO_KHZ800);

// ===== NTP =====
WiFiUDP ntpUDP;
String ntpServer = NTP_SERVER;
long gmtOffset_sec = GMT_OFFSET_SEC;
int daylightOffset_sec = DAYLIGHT_OFFSET_SEC;
String timezoneCity = "berlin";
unsigned long lastTimeUpdate = 0;
int currentHour = 0;
int currentMinute = 0;
int brightness = DEFAULT_BRIGHTNESS; // 1-10
bool testMode = false;
bool firstLEDMode = false;
bool wordEditorActive = false;
unsigned long wordEditorLastActivityMs = 0;
unsigned long diagLastLoopMs = 0;
unsigned long diagLoopCounter = 0;
unsigned long diagLastNtpSyncMs = 0;
unsigned long diagLastWifiDisconnectMs = 0;
int diagLastWifiDisconnectReason = -1;
unsigned long diagLastWifiReconnectAttemptMs = 0;
String diagLastWifiEvent = "boot";

const char* resolveTimezonePosix(const String& city)
{
  if (city == "berlin") return "CET-1CEST,M3.5.0/2,M10.5.0/3";
  if (city == "london") return "GMT0BST,M3.5.0/1,M10.5.0/2";
  if (city == "newyork") return "EST5EDT,M3.2.0/2,M11.1.0/2";
  if (city == "chicago") return "CST6CDT,M3.2.0/2,M11.1.0/2";
  if (city == "denver") return "MST7MDT,M3.2.0/2,M11.1.0/2";
  if (city == "losangeles") return "PST8PDT,M3.2.0/2,M11.1.0/2";
  if (city == "tokyo") return "JST-9";
  if (city == "sydney") return "AEST-10AEDT,M10.1.0/2,M4.1.0/3";
  return "CET-1CEST,M3.5.0/2,M10.5.0/3";
}

void applyTimeConfig()
{
  const char* tz = resolveTimezonePosix(timezoneCity);
  configTzTime(tz, ntpServer.c_str());
  Serial.printf("[TIME] TZ city=%s, tz=%s, ntp=%s\n", timezoneCity.c_str(), tz, ntpServer.c_str());
}

void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      diagLastWifiEvent = "sta_start";
      Serial.println("[WiFi] STA gestartet");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      diagLastWifiEvent = "sta_connected";
      Serial.println("[WiFi] Mit AP verbunden");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      diagLastWifiEvent = "sta_got_ip";
      Serial.println("[WiFi] IP: " + WiFi.localIP().toString());
      applyTimeConfig();
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      diagLastWifiEvent = "sta_disconnected";
      diagLastWifiDisconnectMs = millis();
      diagLastWifiDisconnectReason = static_cast<int>(info.wifi_sta_disconnected.reason);
      Serial.printf("[WiFi] Disconnect, reason=%d\n", diagLastWifiDisconnectReason);
      break;
    default:
      break;
  }
}

bool connectToWiFi(const String &ssid, const String &pass, uint32_t timeoutMs = 15000)
{
  if (ssid.isEmpty())
    return false;

  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs)
    delay(200);

  return (WiFi.status() == WL_CONNECTED);
}

void startAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.println("AP gestartet: " + String(AP_SSID));
  Serial.println("Setup URL: http://" + WiFi.softAPIP().toString());
}

time_t getNTPTime()
{
  time_t now = time(nullptr);
  if (now > 100000) {
    return now;
  }
  return 0;
}

void updateTime()
{
  if (WiFi.status() == WL_CONNECTED) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 1500)) {
      currentHour = timeinfo.tm_hour;
      currentMinute = timeinfo.tm_min;
      lastTimeUpdate = millis();
      diagLastNtpSyncMs = millis();

      // Nachtmodus: 22:00 - 05:59 dimmen
      if (currentHour >= NIGHT_START_HOUR || currentHour < NIGHT_END_HOUR) {
        brightness = NIGHT_BRIGHTNESS;
      } else {
        brightness = DEFAULT_BRIGHTNESS;
      }

      Serial.printf("Zeit aktualisiert: %02d:%02d, Helligkeit: %d, DST=%d\n",
                    currentHour, currentMinute, brightness, timeinfo.tm_isdst);
    } else {
      Serial.println("[TIME] getLocalTime fehlgeschlagen");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(300);
  WiFi.onEvent(onWiFiEvent);

  // LEDs initialisieren
  strip.begin();
  strip.clear();
  strip.show();

  initLetters(&strip);

  // WLAN-Credentials aus Preferences holen
  prefs.begin("wifi", true);
  ssid = prefs.getString("ssid", "");
  pass = prefs.getString("pass", "");
  prefs.end();

  // Settings laden
  prefs.begin("settings", true);
  brightness = prefs.getInt("brightness", DEFAULT_BRIGHTNESS);
  ntpServer = prefs.getString("ntpServer", NTP_SERVER);
  gmtOffset_sec = prefs.getLong("gmtOffset", GMT_OFFSET_SEC);
  daylightOffset_sec = prefs.getInt("daylightOffset", DAYLIGHT_OFFSET_SEC);
  timezoneCity = prefs.getString("timezoneCity", "berlin");
  testMode = prefs.getBool("testMode", false);
  firstLEDMode = prefs.getBool("firstLEDMode", false);
  setLedOrigin(prefs.getInt("ledOrigin", ORIGIN_TOP_LEFT));
  prefs.end();

  // Verbinden oder AP starten
  if (connectToWiFi(ssid, pass))
  {
    Serial.println("WLAN verbunden: " + ssid);
    Serial.println("IP: " + WiFi.localIP().toString());

    // mDNS für lokalen Zugriff
    if (MDNS.begin(HOSTNAME)) {
      Serial.println("mDNS: " + String(HOSTNAME) + ".local");
    } else {
      Serial.println("mDNS-Fehler");
    }

    // OTA nur sinnvoll, wenn WLAN verbunden
    setupOTA(HOSTNAME);
  }
  else
  {
    Serial.println("Kein WLAN -> starte Setup-AP");
    startAP();
  }

  // Webserver starten: WLAN per Webseite speichern (Preferences) + Neustart
  WebServerLogic::begin(server, prefs);

  // erste Zeit holen und anzeigen
  updateTime();
  showTime(currentHour, currentMinute);
  
  // TEST: Erste Reihe leuchten
  testFirstRow();
}

void loop()
{
  diagLastLoopMs = millis();
  diagLoopCounter++;

  // Fallback: lock loest sich, falls Browser ohne "Abbrechen" geschlossen wurde.
  if (wordEditorActive && millis() - wordEditorLastActivityMs > 120000UL) {
    wordEditorActive = false;
    updateTime();
    showTime(currentHour, currentMinute);
  }

  // OTA bedienen (macht nichts, wenn nicht verbunden)
  handleOTA();

  if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED && !ssid.isEmpty()) {
    if (millis() - diagLastWifiReconnectAttemptMs >= 10000) {
      diagLastWifiReconnectAttemptMs = millis();
      Serial.println("[WiFi] Reconnect-Versuch...");
      WiFi.reconnect();
    }
  }

  // Während Word-Editor aktiv ist, darf kein Test- oder Zeitmodus rendern.
  if (wordEditorActive) {
    delay(10);
    return;
  }

  // Im Test-Modus schneller laufen (für Animation)
  if (testMode || firstLEDMode) {
    if (firstLEDMode) {
      showFirstLED();
    } else if (testMode) {
      showAllWords();
    }
  } 
  // Zeit jede Minute aktualisieren in Normal-Mode
  else if (millis() - lastTimeUpdate >= TIME_UPDATE_INTERVAL_MS)
  {
    updateTime();
    showTime(currentHour, currentMinute);
    lastTimeUpdate = millis();
  }

  delay(10);
}
