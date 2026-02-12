#include "logger.h"
#include <Arduino.h>

void logMsg(const char* msg) {
    Serial.println(msg);
}

void logVal(const char* label, int value) {
    Serial.print(label);
    Serial.print(": ");
    Serial.println(value);
}

void logVal(const char* label, const char* value)
{
    Serial.print(label);
    Serial.print(": ");
    Serial.println(value);
}