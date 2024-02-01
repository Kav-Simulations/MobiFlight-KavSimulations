#pragma once

#include <Arduino.h>

uint8_t readFCUPatternFromFlash(uint8_t value);
void getFCUDigitPattern(uint8_t *buffer, uint8_t address, char* digits, uint8_t maxDigits);
