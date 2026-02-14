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
unsigned long lastTimeUpdate = 0;
int currentHour = 0;
int currentMinute = 0;
int brightness = DEFAULT_BRIGHTNESS; // 1-10
bool testMode = false;
bool firstLEDMode = false;
unsigned long diagLastLoopMs = 0;
unsigned long diagLoopCounter = 0;
unsigned long diagLastNtpSyncMs = 0;
unsigned long diagLastWifiDisconnectMs = 0;
int diagLastWifiDisconnectReason = -1;
unsigned long diagLastWifiReconnectAttemptMs = 0;
String diagLastWifiEvent = "boot";

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
  ntpUDP.begin(123);
  const int NTP_PACKET_SIZE = 48;
  byte packetBuffer[NTP_PACKET_SIZE];

  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  ntpUDP.beginPacket(ntpServer.c_str(), 123);
  ntpUDP.write(packetBuffer, NTP_PACKET_SIZE);
  ntpUDP.endPacket();

  delay(1000);

  if (ntpUDP.parsePacket()) {
    ntpUDP.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    time_t epoch = secsSince1900 - seventyYears;
    return epoch + gmtOffset_sec + daylightOffset_sec;
  }
  return 0;
}

void updateTime()
{
  if (WiFi.status() == WL_CONNECTED) {
    time_t now = getNTPTime();
    if (now > 0) {
      struct tm * timeinfo = localtime(&now);
      currentHour = timeinfo->tm_hour;
      currentMinute = timeinfo->tm_min;
      lastTimeUpdate = millis();
      diagLastNtpSyncMs = millis();

      // Nachtmodus: 22:00 - 05:59 dimmen
      if (currentHour >= NIGHT_START_HOUR || currentHour < NIGHT_END_HOUR) {
        brightness = NIGHT_BRIGHTNESS;
      } else {
        brightness = DEFAULT_BRIGHTNESS;
      }

      Serial.printf("Zeit aktualisiert: %02d:%02d, Helligkeit: %d\n", currentHour, currentMinute, brightness);
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

  // OTA bedienen (macht nichts, wenn nicht verbunden)
  handleOTA();

  if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED && !ssid.isEmpty()) {
    if (millis() - diagLastWifiReconnectAttemptMs >= 10000) {
      diagLastWifiReconnectAttemptMs = millis();
      Serial.println("[WiFi] Reconnect-Versuch...");
      WiFi.reconnect();
    }
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
