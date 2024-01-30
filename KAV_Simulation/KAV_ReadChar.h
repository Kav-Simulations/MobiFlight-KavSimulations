#pragma once

#include <Arduino.h>

void displayString(uint8_t *buffer, uint8_t address, char* digits, uint8_t maxDigits, uint8_t dpDigitMask = 0);
