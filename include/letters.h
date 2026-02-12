#pragma once
#include <Adafruit_NeoPixel.h>

void initLetters(Adafruit_NeoPixel* strip);
void clearAll();
uint32_t dimColor(uint8_t r, uint8_t g, uint8_t b);
void lightWord(const char* word, uint32_t color);
void testFirstRow();  // Test: Erste Reihe komplett leuchten lassen
