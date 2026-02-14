#include "letters.h"
#include "matrix.h"
#include "logger.h"
#include <cstring>

static Adafruit_NeoPixel* pixelsPtr = nullptr;

struct WordMap {
    const char* word;
    int leds[LEDS_PER_ROW];
    int count;
    int row;
};

static WordMap words[] = {
    {"ES", {0}, LEN_ES, ROW_1},
    {"IST", {0}, LEN_IST, ROW_1},
    {"FUENF", {0}, LEN_FUENF, ROW_1},

    {"ZEHN", {0}, LEN_ZEHN, ROW_2},
    {"ZWANZIG", {0}, LEN_ZWANZIG, ROW_2},

    {"DREI", {0}, LEN_DREI, ROW_3},
    {"VIERTEL", {0}, LEN_VIERTEL, ROW_3},

    {"VOR", {0}, LEN_VOR, ROW_4},
    {"NACH", {0}, LEN_NACH, ROW_4},

    {"HALB", {0}, LEN_HALB, ROW_5},
    {"ELF", {0}, LEN_ELF, ROW_5},
    {"FUENF_H", {0}, LEN_FUENF_H, ROW_5},

    {"EINS", {0}, LEN_EINS, ROW_6},
    {"ZWEI", {0}, LEN_ZWEI, ROW_6},

    {"DREI_H", {0}, LEN_DREI_H, ROW_7},
    {"VIER", {0}, LEN_VIER, ROW_7},

    {"SECHS", {0}, LEN_SECHS, ROW_8},
    {"ACHT", {0}, LEN_ACHT, ROW_8},

    {"SIEBEN", {0}, LEN_SIEBEN, ROW_9},
    {"ZWOELF", {0}, LEN_ZWOELF, ROW_9},

    {"ZEHN_H", {0}, LEN_ZEHN_H, ROW_10},
    {"NEUN", {0}, LEN_NEUN, ROW_10},
    {"UHR", {0}, LEN_UHR, ROW_10},
};

static constexpr int WORD_COUNT = sizeof(words) / sizeof(words[0]);
static bool wordDefaultsLoaded = false;

static void copyDefault(int *dst, const int *src, int len)
{
    for (int i = 0; i < len; i++) {
        dst[i] = src[i];
    }
}

static void ensureWordDefaults()
{
    if (wordDefaultsLoaded) {
        return;
    }
    wordDefaultsLoaded = true;

    copyDefault(words[0].leds, WORD_ES, LEN_ES);
    copyDefault(words[1].leds, WORD_IST, LEN_IST);
    copyDefault(words[2].leds, WORD_FUENF, LEN_FUENF);
    copyDefault(words[3].leds, WORD_ZEHN, LEN_ZEHN);
    copyDefault(words[4].leds, WORD_ZWANZIG, LEN_ZWANZIG);
    copyDefault(words[5].leds, WORD_DREI, LEN_DREI);
    copyDefault(words[6].leds, WORD_VIERTEL, LEN_VIERTEL);
    copyDefault(words[7].leds, WORD_VOR, LEN_VOR);
    copyDefault(words[8].leds, WORD_NACH, LEN_NACH);
    copyDefault(words[9].leds, WORD_HALB, LEN_HALB);
    copyDefault(words[10].leds, WORD_ELF, LEN_ELF);
    copyDefault(words[11].leds, WORD_FUENF_H, LEN_FUENF_H);
    copyDefault(words[12].leds, WORD_EINS, LEN_EINS);
    copyDefault(words[13].leds, WORD_ZWEI, LEN_ZWEI);
    copyDefault(words[14].leds, WORD_DREI_H, LEN_DREI_H);
    copyDefault(words[15].leds, WORD_VIER, LEN_VIER);
    copyDefault(words[16].leds, WORD_SECHS, LEN_SECHS);
    copyDefault(words[17].leds, WORD_ACHT, LEN_ACHT);
    copyDefault(words[18].leds, WORD_SIEBEN, LEN_SIEBEN);
    copyDefault(words[19].leds, WORD_ZWOELF, LEN_ZWOELF);
    copyDefault(words[20].leds, WORD_ZEHN_H, LEN_ZEHN_H);
    copyDefault(words[21].leds, WORD_NEUN, LEN_NEUN);
    copyDefault(words[22].leds, WORD_UHR, LEN_UHR);
}

static WordMap *findWord(const char *word)
{
    ensureWordDefaults();
    for (auto &entry : words) {
        if (strcmp(entry.word, word) == 0) {
            return &entry;
        }
    }
    return nullptr;
}

void initLetters(Adafruit_NeoPixel* strip) {
    pixelsPtr = strip;
    ensureWordDefaults();
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
    WordMap *entry = findWord(word);
    if (entry) {
        int rowStart = getTopRowStart(entry->row);
        logVal("lightWord", word);

        for (int i = 0; i < entry->count; i++) {
            int pos = entry->leds[i];
            int led = rowStart + pos;
            if (led < 0 || led >= TOTAL_LEDS) {
                continue;
            }
            pixelsPtr->setPixelColor(led, color);
            ledStates[led] = true;
        }
        pixelsPtr->show();
    }
}

int getWordCount()
{
    ensureWordDefaults();
    return WORD_COUNT;
}

const char* getWordNameAt(int index)
{
    ensureWordDefaults();
    if (index < 0 || index >= WORD_COUNT) {
        return nullptr;
    }
    return words[index].word;
}

int getWordLedCountAt(int index)
{
    ensureWordDefaults();
    if (index < 0 || index >= WORD_COUNT) {
        return 0;
    }
    return words[index].count;
}

int getWordLedAt(int index, int pos)
{
    ensureWordDefaults();
    if (index < 0 || index >= WORD_COUNT) {
        return -1;
    }
    if (pos < 0 || pos >= words[index].count) {
        return -1;
    }
    return words[index].leds[pos];
}

bool setWordLedsByName(const char* word, const int* leds, int count)
{
    if (!word || !leds || count <= 0 || count > LEDS_PER_ROW) {
        return false;
    }

    WordMap *entry = findWord(word);
    if (!entry) {
        return false;
    }

    for (int i = 0; i < count; i++) {
        if (leds[i] < 0 || leds[i] >= LEDS_PER_ROW) {
            return false;
        }
    }

    for (int i = 0; i < count; i++) {
        entry->leds[i] = leds[i];
    }
    entry->count = count;
    return true;
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
