#include "timewords.h"
#include "letters.h"
#include "matrix.h"
#include <Arduino.h>

static uint32_t clockWordColor()
{
    return dimColor(255, 255, 255);
}

// Test: Zeigt alle Reihen nacheinander mit Pausen (verwendet Timing)
void testAllRows()
{
    extern Adafruit_NeoPixel strip;
    extern int brightness;
    
    static int currentRow = 0;
    static unsigned long lastRowChange = 0;
    static bool initialized = false;
    const unsigned long ROW_DURATION = 1000; // 1 Sekunde pro Reihe
    
    unsigned long now = millis();
    
    // Initialisiere beim ersten Aufruf
    if (!initialized) {
        lastRowChange = now;
        initialized = true;
    }
    
    // Wechsel zur nächsten Reihe nach ROW_DURATION
    if (now - lastRowChange >= ROW_DURATION) {
        lastRowChange = now;
        
        clearAll();
        
        // Berechne LED-Farbe basierend auf Helligkeit
        float s = brightness / 10.0f;
        uint32_t color = strip.Color(10*s, 10*s, 10*s);
        
        // Alle LEDs der aktuellen Reihe leuchten lassen
        for (int i = 0; i < LEDS_PER_ROW; i++) {
            int led = mapLogicalToPhysical(currentRow, i);
            if (led < 0 || led >= TOTAL_LEDS) {
                continue;
            }
            strip.setPixelColor(led, color);
            ledStates[led] = true;
        }
        
        strip.show();
        
        // Zur nächsten Reihe
        currentRow++;
        if (currentRow >= TOTAL_ROWS) {
            currentRow = 0; // Zurück zur ersten Reihe
        }
    }
}

// Zeigt alle Wörter für LED-Test (veraltet - verwende testAllRows stattdessen)
void showAllWords()
{
    testAllRows();
}

// Zeigt nur die erste LED für Test
void showFirstLED()
{
    clearAll();
    uint32_t color = dimColor(10, 10, 10); // volle Helligkeit für Test

    // Nur die erste LED leuchten lassen
    extern Adafruit_NeoPixel strip;
    strip.setPixelColor(0, color);
    strip.show();
    ledStates[0] = true;
}

// Test: Zeigt alle Wörter gleichzeitig
void showAllWordsAtOnce()
{
    clearAll();
    uint32_t color = dimColor(10, 10, 10); // volle Helligkeit für Test

    // Alle Wörter aus der Matrix leuchten lassen
    lightWord("ES", color);
    lightWord("IST", color);
    lightWord("FUENF", color);
    lightWord("ZEHN", color);
    lightWord("ZWANZIG", color);
    lightWord("DREI", color);
    lightWord("VIERTEL", color);
    lightWord("VOR", color);
    lightWord("NACH", color);
    lightWord("HALB", color);
    lightWord("ELF", color);
    lightWord("FUENF_H", color);
    lightWord("EINS", color);
    lightWord("ZWEI", color);
    lightWord("DREI_H", color);
    lightWord("VIER", color);
    lightWord("SECHS", color);
    lightWord("ACHT", color);
    lightWord("SIEBEN", color);
    lightWord("NEUN", color);
    lightWord("ZWOELF", color);
    lightWord("ZEHN_H", color);
    lightWord("UHR", color);
}

// Hilfsfunktion: Stunde als Wort anzeigen
static void showHour(int hour)
{
    hour = hour % 12;
    if (hour == 0) hour = 12;

    switch (hour) {
        case 1:  lightWord("EINS", clockWordColor()); break;
        case 2:  lightWord("ZWEI", clockWordColor()); break;
        case 3:  lightWord("DREI_H", clockWordColor()); break;
        case 4:  lightWord("VIER", clockWordColor()); break;
        case 5:  lightWord("FUENF_H", clockWordColor()); break;
        case 6:  lightWord("SECHS", clockWordColor()); break;
        case 7:  lightWord("SIEBEN", clockWordColor()); break;
        case 8:  lightWord("ACHT", clockWordColor()); break;
        case 9:  lightWord("NEUN", clockWordColor()); break;
        case 10: lightWord("ZEHN_H", clockWordColor()); break;
        case 11: lightWord("ELF", clockWordColor()); break;
        case 12: lightWord("ZWOELF", clockWordColor()); break;
    }
}

// Hauptfunktion
void showTime(int hour, int minute)
{
    clearAll();

    // ES IST
    lightWord("ES", clockWordColor());
    lightWord("IST", clockWordColor());

    if (minute < 5) {
        showHour(hour);
        lightWord("UHR", clockWordColor());
        return;
    }

    if (minute < 10) {
        lightWord("FUENF", clockWordColor());
        lightWord("NACH", clockWordColor());
        showHour(hour);
    }
    else if (minute < 15) {
        lightWord("ZEHN", clockWordColor());
        lightWord("NACH", clockWordColor());
        showHour(hour);
    }
    else if (minute < 20) {
        lightWord("VIERTEL", clockWordColor());
        lightWord("NACH", clockWordColor());
        showHour(hour);
    }
    else if (minute < 25) {
        lightWord("ZWANZIG", clockWordColor());
        lightWord("NACH", clockWordColor());
        showHour(hour);
    }
    else if (minute < 30) {
        lightWord("FUENF", clockWordColor());
        lightWord("VOR", clockWordColor());
        lightWord("HALB", clockWordColor());
        showHour(hour + 1);
    }
    else if (minute < 35) {
        lightWord("HALB", clockWordColor());
        showHour(hour + 1);
    }
    else if (minute < 40) {
        lightWord("FUENF", clockWordColor());
        lightWord("NACH", clockWordColor());
        lightWord("HALB", clockWordColor());
        showHour(hour + 1);
    }
    else if (minute < 45) {
        lightWord("ZWANZIG", clockWordColor());
        lightWord("VOR", clockWordColor());
        showHour(hour + 1);
    }
    else if (minute < 50) {
        lightWord("VIERTEL", clockWordColor());
        lightWord("VOR", clockWordColor());
        showHour(hour + 1);
    }
    else if (minute < 55) {
        lightWord("ZEHN", clockWordColor());
        lightWord("VOR", clockWordColor());
        showHour(hour + 1);
    }
    else {
        lightWord("FUENF", clockWordColor());
        lightWord("VOR", clockWordColor());
        showHour(hour + 1);
    }
}
