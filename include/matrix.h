#pragma once

// ===== Matrix-Grunddaten =====
constexpr int LEDS_PER_ROW   = 41;  // LEDs pro physische Reihe
constexpr int ROWS_PER_WORD  = 1;   // 1 physische Reihe pro Wortreihe
constexpr int WORD_ROWS      = 10;  // 10 Wortzeilen

constexpr int TOTAL_ROWS  = WORD_ROWS * ROWS_PER_WORD;  // 10 physische Reihen
constexpr int TOTAL_LEDS  = TOTAL_ROWS * LEDS_PER_ROW;  // 410 LEDs gesamt

// Helligkeit 1..10
constexpr int BRIGHTNESS_LEVEL = 10;

// LED-Zustände für Live-Update
extern bool ledStates[TOTAL_LEDS];

enum LedOrigin
{
    ORIGIN_TOP_LEFT = 0,
    ORIGIN_TOP_RIGHT = 1,
    ORIGIN_BOTTOM_LEFT = 2,
    ORIGIN_BOTTOM_RIGHT = 3
};

extern LedOrigin ledOrigin;

// Wortreihen
constexpr int ROW_1 = 0;
constexpr int ROW_2 = 1;
constexpr int ROW_3 = 2;
constexpr int ROW_4 = 3;
constexpr int ROW_5 = 4;
constexpr int ROW_6 = 5;
constexpr int ROW_7 = 6;
constexpr int ROW_8 = 7;
constexpr int ROW_9 = 8;
constexpr int ROW_10 = 9;

// Funktionen
int getTotalLEDs();
int getTopRowStart(int wordRow);
int mapLogicalToPhysical(int logicalRow, int logicalCol);
void setLedOrigin(int origin);
int getLedOrigin();
const char* getLedOriginName();

// ===== Wortdefinitionen =====

// Reihe 1
extern const int WORD_ES[];      extern const int LEN_ES;
extern const int WORD_IST[];     extern const int LEN_IST;
extern const int WORD_FUENF[];   extern const int LEN_FUENF;

// Reihe 2
extern const int WORD_ZEHN[];    extern const int LEN_ZEHN;
extern const int WORD_ZWANZIG[]; extern const int LEN_ZWANZIG;

// Reihe 3
extern const int WORD_DREI[];    extern const int LEN_DREI;
extern const int WORD_VIERTEL[]; extern const int LEN_VIERTEL;

// Reihe 4
extern const int WORD_VOR[];     extern const int LEN_VOR;
extern const int WORD_NACH[];    extern const int LEN_NACH;

// Reihe 5
extern const int WORD_HALB[];    extern const int LEN_HALB;
extern const int WORD_ELF[];     extern const int LEN_ELF;
extern const int WORD_FUENF_H[]; extern const int LEN_FUENF_H;

// Reihe 6
extern const int WORD_EINS[];    extern const int LEN_EINS;
extern const int WORD_ZWEI[];    extern const int LEN_ZWEI;

// Reihe 7
extern const int WORD_DREI_H[];  extern const int LEN_DREI_H;
extern const int WORD_VIER[];    extern const int LEN_VIER;

// Reihe 8
extern const int WORD_SECHS[];   extern const int LEN_SECHS;
extern const int WORD_ACHT[];    extern const int LEN_ACHT;

// Reihe 9
extern const int WORD_SIEBEN[];  extern const int LEN_SIEBEN;
extern const int WORD_NEUN[];    extern const int LEN_NEUN;
extern const int WORD_ZWOELF[];  extern const int LEN_ZWOELF;

// Reihe 10
extern const int WORD_ZEHN_H[];  extern const int LEN_ZEHN_H;
extern const int WORD_UHR[];     extern const int LEN_UHR;
