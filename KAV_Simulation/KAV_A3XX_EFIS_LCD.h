/**
 * Kav A3XX EFIS LCD
 * Written by: James Kavanagh & Keith Greenwood
 * This library has been written to drive the 'Kav Simulations' EFIS LCD Display.
 */

#pragma once

#include "Arduino.h"
#include "HT1621.h"

#define BUFFER_SIZE_MAX 16

class KAV_A3XX_EFIS_LCD
{
private:
    // Fields
    HT1621   ht_efis;
    uint8_t  buffer[BUFFER_SIZE_MAX];
    bool     _initialised;
    byte     _CS;
    byte     _CLK;
    byte     _DATA;
    uint16_t _value = 0;
    uint8_t  _mode;

    // Methods
    void displayDigit(uint8_t address, uint8_t digit);
    void refreshLCD(uint8_t address);

public:
    // Constructor
    // 'CLK' is sometimes referred to as 'RW'
    KAV_A3XX_EFIS_LCD(uint8_t CS, uint8_t CLK, uint8_t DATA) : ht_efis(CS, CLK, DATA){};

    void begin();
    void clearLCD();
    void attach(byte CS, byte CLK, byte DATA);
    void detach();
    void set(int16_t messageID, char *setPoint);

    // Set QFE or QNH functions
    void setQFE(bool enabled);
    void setQNH(bool enabled);
    // Set Dot Function
    void setDot(bool enabled);
    // Show Std function
    void showStd();

    // Show value, either QFE or QNH
    void showValue(uint16_t value);
};
