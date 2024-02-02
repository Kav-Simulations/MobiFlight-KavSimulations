/**
 * KAV A3XX FCU LCD
 * Written by: James Kavanagh & Keith Greenwood
 * This library has been written to drive the 'Kav Simulations' FCU LCD Display.
 */

#pragma once

#include "Arduino.h"
#include "HT1621.h"

#define BUFFER_SIZE_MAX 16

class KAV_A3XX_FCU_LCD
{
private:
    // Fields
    HT1621  ht;
    uint8_t buffer[BUFFER_SIZE_MAX];
    bool    vertSignEnabled;
    bool    _initialised;
    byte    _CS;
    byte    _CLK;
    byte    _DATA;
    bool    trkActive;

    // Methods
    void displayDigit(uint8_t address, uint8_t digit);
    void refreshLCD(uint8_t address);
    void refreshLCD(uint8_t address, uint8_t digits);

public:
    // Constructor
    // 'CLK' is sometimes referred to as 'RW'
    KAV_A3XX_FCU_LCD(uint8_t CS, uint8_t CLK, uint8_t DATA) : ht(CS, CLK, DATA), vertSignEnabled(true){};

    void begin();
    void clearLCD();
    void attach(byte CS, byte CLK, byte DATA);
    void detach();
    void set(int16_t messageID, char *setPoint);

    // Speed and Mach functions
    void setSpeedDashes(int8_t state);
    void setSpeedLabel(bool enabled);
    void setMachLabel(bool enabled);
    void setSpeedDot(int8_t state);
    // Speed and Mach with setting labels
    void setSpeedMode(int16_t value);
    void setMachMode(int16_t value);
    void setMachMode(float value);
    // Speed and Mach without setting labels
    void showSpeedValue(uint16_t value);
    void showMachValue(uint16_t value);
    void showSpeedMachValue(float value);
    void showSpeedMachValue(char* data);
    void toggleSpeedMachMode(int8_t state);

    // Heading, Track and Latitude Functions
    void setHeadingDashes(int8_t state);
    void setHeadingLabel(bool enabled);
    void setTrackLabel(bool enabled);
    void setLatitudeLabel(bool enabled);
    void setHeadingDot(int8_t state);
    void showHeadingValue(int16_t value);
    void showHeadingValue(char* data);

    // Altitude Functions
    void setAltitudeDashes(int8_t state);
    void setAltitudeLabel(bool enabled);
    void setLvlChLabel(bool enabled);
    void setAltitudeDot(int8_t state);
    void showAltitudeValue(uint32_t value);
    void showAltitudeValue(float value);
    void showAltitudeValue(char* data);

    // Vertical_Speed & FPA Functions
    void setVrtSpdDashes(int8_t state);
    void setVrtSpdLabel(bool enabled);
    void setFPALabel(bool enabled);
    void setSignLabel(bool enabled);
    void showVerticalValue(int16_t value);
    void showVerticalValue(float value);
    void showFPAValue(int8_t value);
    void showFPAValue(float value);
    void showVerticalFPAValue(char* data);

    // Preset States
    void setStartLabels();
    void toggleTrkHdgMode(int8_t state);
    void setHeadingMode();
    void setTrackMode();
    void clearOrReset(bool enabled);
};
