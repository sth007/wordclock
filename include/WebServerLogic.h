#pragma once

#include <ESPAsyncWebServer.h>
#include <Preferences.h>

extern int brightness;

namespace WebServerLogic
{
  // Server-Routen registrieren + starten
  void begin(AsyncWebServer &server, Preferences &prefs);

  // Optional: Helfer, falls du HTML-Dateien später erweitern willst
  bool isFsReady();
}
