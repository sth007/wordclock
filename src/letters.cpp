#include "letters.h"
#include "matrix.h"
#include "logger.h"
#include <cstring>

static Adafruit_NeoPixel* pixelsPtr = nullptr;

struct WordMap {
    const char* word;
    const int* leds;
    int count;
    int row;
};

static WordMap words[] = {
    {"ES", WORD_ES, LEN_ES, ROW_1},
    {"IST", WORD_IST, LEN_IST, ROW_1},
    {"FUENF", WORD_FUENF, LEN_FUENF, ROW_1},

    {"ZEHN", WORD_ZEHN, LEN_ZEHN, ROW_2},
    {"ZWANZIG", WORD_ZWANZIG, LEN_ZWANZIG, ROW_2},

    {"DREI", WORD_DREI, LEN_DREI, ROW_3},
    {"VIERTEL", WORD_VIERTEL, LEN_VIERTEL, ROW_3},

    {"VOR", WORD_VOR, LEN_VOR, ROW_4},
    {"NACH", WORD_NACH, LEN_NACH, ROW_4},

    {"HALB", WORD_HALB, LEN_HALB, ROW_5},
    {"ELF", WORD_ELF, LEN_ELF, ROW_5},
    {"FUENF_H", WORD_FUENF_H, LEN_FUENF_H, ROW_5},

    {"EINS", WORD_EINS, LEN_EINS, ROW_6},
    {"ZWEI", WORD_ZWEI, LEN_ZWEI, ROW_6},

    {"DREI_H", WORD_DREI_H, LEN_DREI_H, ROW_7},
    {"VIER", WORD_VIER, LEN_VIER, ROW_7},

    {"SECHS", WORD_SECHS, LEN_SECHS, ROW_8},
    {"ACHT", WORD_ACHT, LEN_ACHT, ROW_8},

    {"SIEBEN", WORD_SIEBEN, LEN_SIEBEN, ROW_9},
    {"NEUN", WORD_NEUN, LEN_NEUN, ROW_9},
    {"ZWOELF", WORD_ZWOELF, LEN_ZWOELF, ROW_9},

    {"ZEHN_H", WORD_ZEHN_H, LEN_ZEHN_H, ROW_10},
    {"UHR", WORD_UHR, LEN_UHR, ROW_10},
};

void initLetters(Adafruit_NeoPixel* strip) {
    pixelsPtr = strip;
}

void clearAll() {
    pixelsPtr->clear();
    pixelsPtr->show();
    memset(ledStates, 0, sizeof(ledStates)); // Alle LEDs aus
}

uint32_t dimColor(uint8_t r, uint8_t g, uint8_t b) {
    extern int brightness;
    float s = brightness / 10.0f;
    return pixelsPtr->Color(r*s, g*s, b*s);
}

void lightWord(const char* word, uint32_t color) {
    for (auto& e : words) {
        if (strcmp(e.word, word) == 0) {
            int topStart = getTopRowStart(e.row);
            logVal("lightWord", word);

            for (int i = 0; i < e.count; i++) {
                int pos = e.leds[i];
                int top = topStart + pos;
                int bottom = topStart + LEDS_PER_ROW + (LEDS_PER_ROW - 1 - pos);

                pixelsPtr->setPixelColor(top, color);
                pixelsPtr->setPixelColor(bottom, color);
                ledStates[top] = true;
                ledStates[bottom] = true;
            }
            pixelsPtr->show();
            return;
        }
    }
}

void testFirstRow() {
    // Test: Erste Reihe mit reduzierter Helligkeit und Delays
    clearAll();
    uint32_t color = pixelsPtr->Color(80, 80, 80);  // Weiß, aber gedimmt (Stromversorgung)
    
    // Setze alle 41 LEDs mit Delays zwischen setPixelColor() Aufrufen
    for (int i = 0; i < LEDS_PER_ROW; i++) {
        pixelsPtr->setPixelColor(i, color);
        ledStates[i] = true;
        delay(1);  // 1ms Verzögerung um Stromspitzen zu vermeiden
    }
    
    pixelsPtr->show();
    Serial.printf("Test: %d LEDs mit Helligkeit 80/255 gesteuert\n", LEDS_PER_ROW);
}
