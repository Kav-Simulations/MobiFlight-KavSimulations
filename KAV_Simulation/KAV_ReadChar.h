#pragma once

#include <Arduino.h>

uint8_t readFCUCharFromFlash(uint8_t value);
void displayStringFCU(uint8_t *buffer, uint8_t address, char* digits, uint8_t maxDigits, uint8_t dpDigitMask = 0);