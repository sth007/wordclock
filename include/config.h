#pragma once

// ===== Hostname für OTA und mDNS =====
#define HOSTNAME "wordclock"

// ===== NTP =====
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 3600      // GMT+1
#define DAYLIGHT_OFFSET_SEC 3600 // Sommerzeit

// ===== LED =====
#define LED_PIN 8

// ===== AP =====
#define AP_SSID "ESP32-Setup"
#define AP_PASS ""

// ===== Update-Intervall =====
#define TIME_UPDATE_INTERVAL_MS 60000UL // 1 Minute

// ===== Helligkeit =====
#define DEFAULT_BRIGHTNESS 10
#define NIGHT_BRIGHTNESS 2
#define NIGHT_START_HOUR 22
#define NIGHT_END_HOUR 6