/**
 * KAV A3XX Digit Pattern
 * Written by: James Kavanagh & Ralf (@elral)
 */

#pragma once

#include <Arduino.h>

void getFCUDigitPattern(uint8_t *buffer, uint8_t address, char* digits, uint8_t maxDigits, uint8_t dpMask = 0);
void getDigitPattern(uint8_t *buffer, uint8_t address, char* digits, uint8_t maxDigits, uint8_t dpDigitMask = 0);
