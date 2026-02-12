#include "matrix.h"

// LED-Zustände initialisieren
bool ledStates[TOTAL_LEDS] = {false};

// ===== Funktionen =====
int getTotalLEDs() {
    return TOTAL_LEDS;
}

int getTopRowStart(int wordRow) {
    return wordRow * LEDS_PER_ROW * ROWS_PER_WORD;
}

// ===== Wortdefinitionen =====

// Reihe 1
const int WORD_ES[]    = {0,1,2,3};                                             const int LEN_ES = 4;
const int WORD_IST[]   = {6,7,8,9,10,11};                                       const int LEN_IST = 6;
const int WORD_FUENF[] = {15,16,17,18,19,20,21,22};                             const int LEN_FUENF = 8;

// Reihe 2
const int WORD_ZEHN[]    = {0,1,2,3,4,5,6,7,8};                                 const int LEN_ZEHN = 9;
const int WORD_ZWANZIG[] = {9,10,11,12,13,14,15,16,17,18,19,20,21,22};          const int LEN_ZWANZIG = 14;

// Reihe 3
const int WORD_DREI[]    = {0,1,2,3,4,5};                                       const int LEN_DREI = 6;
const int WORD_VIERTEL[] = {6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22};    const int LEN_VIERTEL = 17;

// Reihe 4
const int WORD_VOR[]  = {0,1,2,3,4,5,6};                                        const int LEN_VOR = 7;
const int WORD_NACH[] = {15,16,17,18,19,20,21,22};                              const int LEN_NACH = 8;

// Reihe 5
const int WORD_HALB[]    = {0,1,2,3,4,5,6};                                     const int LEN_HALB = 7;
const int WORD_ELF[]     = {11,12,13};                                          const int LEN_ELF = 3;
const int WORD_FUENF_H[] = {15,16,17,18,19,20,21,22};                           const int LEN_FUENF_H = 8;

// Reihe 6
const int WORD_EINS[] = {0,1,2,3};                                              const int LEN_EINS = 4;
const int WORD_ZWEI[] = {6,7,8,9};                                              const int LEN_ZWEI = 4;

// Reihe 7
const int WORD_DREI_H[] = {0,1,2,3};                                            const int LEN_DREI_H = 4;
const int WORD_VIER[]   = {6,7,8,9};                                            const int LEN_VIER = 4;

// Reihe 8
const int WORD_SECHS[] = {0,1,2,3,4};                                           const int LEN_SECHS = 5;
const int WORD_ACHT[]  = {7,8,9,10};                                            const int LEN_ACHT = 4;

// Reihe 9
const int WORD_SIEBEN[] = {0,1,2,3,4,5};                                        const int LEN_SIEBEN = 6;
const int WORD_NEUN[]   = {7,8,9,10};                                          const int LEN_NEUN = 4;
const int WORD_ZWOELF[] = {12,13,14,15,16,17};                                 const int LEN_ZWOELF = 6;

// Reihe 10
const int WORD_ZEHN_H[] = {0,1,2,3};                                            const int LEN_ZEHN_H = 4;
const int WORD_UHR[]    = {6,7,8};                                              const int LEN_UHR = 3;
