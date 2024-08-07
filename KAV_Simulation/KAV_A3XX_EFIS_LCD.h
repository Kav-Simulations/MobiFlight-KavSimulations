/**
 * KAV A3XX EFIS LCD
 * Written by: James Kavanagh, Ralf (@elral), & Keith Greenwood
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
    HT1621  ht_efis;
    uint8_t buffer[BUFFER_SIZE_MAX];
    bool    _initialised;
    byte    _CS;
    byte    _CLK;
    byte    _DATA;
    bool    _lastQNHlabel = false;
    bool    _lastQFElabel = false;
    char    _lastQFEQNHValue[10] = {};

    // Methods
    void displayDigit(uint8_t address, uint8_t digit);
    void refreshLCD(uint8_t address);
    void refreshLCD(uint8_t address, uint8_t digits);

public:
    // Constructor
    // 'CLK' is sometimes referred to as 'RW'
    KAV_A3XX_EFIS_LCD(uint8_t CS, uint8_t CLK, uint8_t DATA) : ht_efis(CS, CLK, DATA){};

    void begin();
    void clearLCD();
    void attach(byte CS, byte CLK, byte DATA);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void setPowerSave(bool enabled);

    // Set QFE or QNH functions
    void setQFElabel(bool enabled);
    void setQNHlabel(bool enabled);
    
    // Show Std function
    void showStd(uint16_t state);

    // Show QFE function, labels get set
    void showQFEValue(uint16_t value);
    void showQFEValue(float value);
    void showQFEValue(char* value);
    // Show QNH function, labels get set
    void showQNHValue(uint16_t value);
    void showQNHValue(float value);
    void showQNHValue(char* value);

    // Show QFE or QNH without setting labels
    void showQFEQNHValue(uint16_t value);
    void showQFEQNHValue(float value);
    void showQFEQNHValue(char* value);

    // Annunciator Test Function
    void setAnnunciatorTest(bool enabled);
};
