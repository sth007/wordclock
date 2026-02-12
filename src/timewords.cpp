#include "timewords.h"
#include "letters.h"
#include "matrix.h"
#include <Arduino.h>

// Zeigt alle Wörter für LED-Test
void showAllWords()
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

// Hilfsfunktion: Stunde als Wort anzeigen
static void showHour(int hour)
{
    hour = hour % 12;
    if (hour == 0) hour = 12;

    switch (hour) {
        case 1:  lightWord("EINS", dimColor(20,20,20)); break;
        case 2:  lightWord("ZWEI", dimColor(20,20,20)); break;
        case 3:  lightWord("DREI_H", dimColor(20,20,20)); break;
        case 4:  lightWord("VIER", dimColor(20,20,20)); break;
        case 5:  lightWord("FUENF_H", dimColor(20,20,20)); break;
        case 6:  lightWord("SECHS", dimColor(20,20,20)); break;
        case 7:  lightWord("SIEBEN", dimColor(20,20,20)); break;
        case 8:  lightWord("ACHT", dimColor(20,20,20)); break;
        case 9:  lightWord("NEUN", dimColor(20,20,20)); break;
        case 10: lightWord("ZEHN_H", dimColor(20,20,20)); break;
        case 11: lightWord("ELF", dimColor(20,20,20)); break;
        case 12: lightWord("ZWOELF", dimColor(20,20,20)); break;
    }
}

// Hauptfunktion
void showTime(int hour, int minute)
{
    clearAll();

    // ES IST
    lightWord("ES", dimColor(20,20,20));
    lightWord("IST", dimColor(20,20,20));

    if (minute < 5) {
        showHour(hour);
        lightWord("UHR", dimColor(20,20,20));
        return;
    }

    if (minute < 10) {
        lightWord("FUENF", dimColor(20,20,20));
        lightWord("NACH", dimColor(20,20,20));
        showHour(hour);
    }
    else if (minute < 15) {
        lightWord("ZEHN", dimColor(20,20,20));
        lightWord("NACH", dimColor(20,20,20));
        showHour(hour);
    }
    else if (minute < 20) {
        lightWord("VIERTEL", dimColor(20,20,20));
        lightWord("NACH", dimColor(20,20,20));
        showHour(hour);
    }
    else if (minute < 25) {
        lightWord("ZWANZIG", dimColor(20,20,20));
        lightWord("NACH", dimColor(20,20,20));
        showHour(hour);
    }
    else if (minute < 30) {
        lightWord("FUENF", dimColor(20,20,20));
        lightWord("VOR", dimColor(20,20,20));
        lightWord("HALB", dimColor(20,20,20));
        showHour(hour + 1);
    }
    else if (minute < 35) {
        lightWord("HALB", dimColor(20,20,20));
        showHour(hour + 1);
    }
    else if (minute < 40) {
        lightWord("FUENF", dimColor(20,20,20));
        lightWord("NACH", dimColor(20,20,20));
        lightWord("HALB", dimColor(20,20,20));
        showHour(hour + 1);
    }
    else if (minute < 45) {
        lightWord("ZWANZIG", dimColor(20,20,20));
        lightWord("VOR", dimColor(20,20,20));
        showHour(hour + 1);
    }
    else if (minute < 50) {
        lightWord("VIERTEL", dimColor(20,20,20));
        lightWord("VOR", dimColor(20,20,20));
        showHour(hour + 1);
    }
    else if (minute < 55) {
        lightWord("ZEHN", dimColor(20,20,20));
        lightWord("VOR", dimColor(20,20,20));
        showHour(hour + 1);
    }
    else {
        lightWord("FUENF", dimColor(20,20,20));
        lightWord("VOR", dimColor(20,20,20));
        showHour(hour + 1);
    }
}
