#include "matrix.h"

// LED-Zustände initialisieren
bool ledStates[TOTAL_LEDS] = {false};
LedOrigin ledOrigin = ORIGIN_TOP_LEFT;

// Makro um Arraygröße automatisch zu berechnen
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

// ===== Funktionen =====
int getTotalLEDs() {
    return TOTAL_LEDS;
}

int getTopRowStart(int wordRow) {
    return wordRow * LEDS_PER_ROW * ROWS_PER_WORD;
}

int mapLogicalToPhysical(int logicalRow, int logicalCol)
{
    if (logicalRow < 0 || logicalRow >= TOTAL_ROWS || logicalCol < 0 || logicalCol >= LEDS_PER_ROW) {
        return -1;
    }

    // k = Reihenindex in Verdrahtungsreihenfolge (0 = erste physisch verkabelte Reihe)
    const bool startsFromTop = (ledOrigin == ORIGIN_TOP_LEFT || ledOrigin == ORIGIN_TOP_RIGHT);
    const bool startsFromLeft = (ledOrigin == ORIGIN_TOP_LEFT || ledOrigin == ORIGIN_BOTTOM_LEFT);
    int k = startsFromTop ? logicalRow : (TOTAL_ROWS - 1 - logicalRow);

    // Zig-zag: jede nächste Reihe läuft entgegengesetzt.
    bool rowGoesLeftToRight = startsFromLeft;
    if ((k % 2) == 1) {
        rowGoesLeftToRight = !rowGoesLeftToRight;
    }

    int physicalCol = rowGoesLeftToRight ? logicalCol : (LEDS_PER_ROW - 1 - logicalCol);
    return k * LEDS_PER_ROW + physicalCol;
}

void setLedOrigin(int origin)
{
    if (origin < ORIGIN_TOP_LEFT || origin > ORIGIN_BOTTOM_RIGHT) {
        return;
    }
    ledOrigin = static_cast<LedOrigin>(origin);
}

int getLedOrigin()
{
    return static_cast<int>(ledOrigin);
}

const char* getLedOriginName()
{
    switch (ledOrigin) {
    case ORIGIN_TOP_RIGHT:
        return "oben_rechts";
    case ORIGIN_BOTTOM_LEFT:
        return "unten_links";
    case ORIGIN_BOTTOM_RIGHT:
        return "unten_rechts";
    case ORIGIN_TOP_LEFT:
    default:
        return "oben_links";
    }
}

// ===== Wortdefinitionen =====

// Reihe 1
const int WORD_ES[]    = {0,1,2,3,4,5,6};
const int LEN_ES = ARRAY_LEN(WORD_ES);
const int WORD_IST[]   = {6,7,8,9,10,11};
const int LEN_IST = ARRAY_LEN(WORD_IST);
const int WORD_FUENF[] = {15,16,17,18,19,20,21,22};
const int LEN_FUENF = ARRAY_LEN(WORD_FUENF);

// Reihe 2
const int WORD_ZEHN[]    = {0,1,2,3,4,5,6,7,8};
const int LEN_ZEHN = ARRAY_LEN(WORD_ZEHN);
const int WORD_ZWANZIG[] = {9,10,11,12,13,14,15,16,17,18,19,20,21,22};
const int LEN_ZWANZIG = ARRAY_LEN(WORD_ZWANZIG);

// Reihe 3
const int WORD_DREI[]    = {0,1,2,3,4,5};
const int LEN_DREI = ARRAY_LEN(WORD_DREI);
const int WORD_VIERTEL[] = {6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22};
const int LEN_VIERTEL = ARRAY_LEN(WORD_VIERTEL);

// Reihe 4
const int WORD_VOR[]  = {0,1,2,3,4,5,6};
const int LEN_VOR = ARRAY_LEN(WORD_VOR);
const int WORD_NACH[] = {15,16,17,18,19,20,21,22};
const int LEN_NACH = ARRAY_LEN(WORD_NACH);

// Reihe 5
const int WORD_HALB[]    = {0,1,2,3,4,5,6};
const int LEN_HALB = ARRAY_LEN(WORD_HALB);
const int WORD_ELF[]     = {11,12,13};
const int LEN_ELF = ARRAY_LEN(WORD_ELF);
const int WORD_FUENF_H[] = {15,16,17,18,19,20,21,22};
const int LEN_FUENF_H = ARRAY_LEN(WORD_FUENF_H);

// Reihe 6
const int WORD_EINS[] = {0,1,2,3};
const int LEN_EINS = ARRAY_LEN(WORD_EINS);
const int WORD_ZWEI[] = {6,7,8,9};
const int LEN_ZWEI = ARRAY_LEN(WORD_ZWEI);

// Reihe 7
const int WORD_DREI_H[] = {0,1,2,3};
const int LEN_DREI_H = ARRAY_LEN(WORD_DREI_H);
const int WORD_VIER[]   = {6,7,8,9};
const int LEN_VIER = ARRAY_LEN(WORD_VIER);

// Reihe 8
const int WORD_SECHS[] = {0,1,2,3,4};
const int LEN_SECHS = ARRAY_LEN(WORD_SECHS);
const int WORD_ACHT[]  = {7,8,9,10};
const int LEN_ACHT = ARRAY_LEN(WORD_ACHT);

// Reihe 9
const int WORD_SIEBEN[] = {0,1,2,3,4,5};
const int LEN_SIEBEN = ARRAY_LEN(WORD_SIEBEN);
const int WORD_NEUN[]   = {7,8,9,10};
const int LEN_NEUN = ARRAY_LEN(WORD_NEUN);
const int WORD_ZWOELF[] = {12,13,14,15,16,17};
const int LEN_ZWOELF = ARRAY_LEN(WORD_ZWOELF);

// Reihe 10
const int WORD_ZEHN_H[] = {0,1,2,3};
const int LEN_ZEHN_H = ARRAY_LEN(WORD_ZEHN_H);
const int WORD_UHR[]    = {6,7,8};
const int LEN_UHR = ARRAY_LEN(WORD_UHR);
