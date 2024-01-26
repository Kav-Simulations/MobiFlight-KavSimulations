/**
 * KAV A3XX FCU LCD v2.0
 * Written by: James Kavanagh & Keith Greenwood
 * Additional contributions from github.com/elral
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
    bool    _initialised;
    byte    _CS;
    byte    _CLK;
    byte    _DATA;

    // Methods
    void displayDigit(uint8_t address, uint8_t digit);
    void refreshLCD(uint8_t address);

public:
    // Constructor and Setup
    // 'CLK' is sometimes referred to as 'RW'
    KAV_A3XX_FCU_LCD(uint8_t CS, uint8_t CLK, uint8_t DATA) : ht(CS, CLK, DATA) {};
    void begin();
    void clearLCD();
    void attach(byte CS, byte CLK, byte DATA);
    void detach();
    void set(int16_t messageID, char *setPoint);

    // DISPLAY FUNCTIONS //
    void setStartLabels(bool enabled);
    void setTrkMode();
    void setHdgMode();

    // Speed and Mach Functions
    void setSpdLabel(bool enabled);
    void setMachLabel(bool enabled);
    void setSpdValueInt(uint16_t value);
    void setSpdValueFloat(float value);
    void setSpdDot(bool enabled);
    void setSpdDashes(bool enabled);

    // Heading and Track Functions
    void setLatLabel(bool enabled);
    void setHdgLabel(bool enabled);
    void setTrkLabel(bool enabled);
    void setHdgValue(uint16_t value);
    void setHdgDot(bool enabled);
    void setHdgDashes(bool enabled);

    // Centre Display Functions
    void setHdgVsLabel(bool enabled);
    void setTrkFpaLabel(bool enabled);

    // Altitude Functions
    void setLvlChLabel(bool enabled);
    void setAltLabel(bool enabled);
    void setAltValue(uint32_t value);
    void setAltDot(bool enabled);
    void setAltDashes(bool enabled);

    // Vertical Speed Functions
    void setVsLabel(bool enabled);
    void setFpaLabel(bool enabled);
    void setVsValue(int16_t value);
    void setFpaValueInt(int16_t value);
    void setFpaValueFloat(float value);
    void setPlus(bool enabled);
    void setMinus(bool enabled);
    void setVsDashes(bool enabled);

    // Presets and Combined Functions
    void showSPDInt(char* data);
    void showSPDFloat(char* data);
    // void showHDG(uint16_t value);
    // void showALT(uint32_t value);
    void showVSInt(char* data);
    void showVSFloat(char* data);
    void showVSInt_PlusMinus(char* data);
    void showVSFloat_PlusMinus(char* data);
    void toggleTrkHdgMode(bool enabled);
    void testMode(bool enabled);



    // // Speed and Mach functions
    // void setSpeedLabel(bool enabled);
    // void setMachLabel(bool enabled);
    // void setSpeedDot(int8_t state);
    // void showSpeedValue(uint16_t value);

    // // Heading, Track and Latitude Functions
    // void setHeadingLabel(bool enabled);
    // void setTrackLabel(bool enabled);
    // void setLatitudeLabel(bool enabled);
    // void setHeadingDot(int8_t state);
    // void showHeadingValue(uint16_t value);

    // // Altitude Functions
    // void setAltitudeLabel(bool enabled);
    // void setLvlChLabel(bool enabled);
    // void setAltitudeDot(int8_t state);
    // void showAltitudeValue(uint32_t value);

    // // Vertical_Speed & FPA Functions
    // void setVrtSpdLabel(bool enabled);
    // void setFPALabel(bool enabled);
    // void setSignLabel(bool enabled);
    // void showVerticalValue(int16_t value);
    // void showFPAValue(int8_t value);

    // // Preset States
    // void setSpeedDashes(int8_t state);
    // void setHeadingDashes(int8_t state);
    // void setAltitudeDashes(int8_t state);
    // void setVrtSpdDashes(int8_t state);
    // void setStartLabels();
    // void toggleTrkHdgMode(int8_t state);
    // void setHeadingMode();
    // void setTrackMode();
    // void setSpeedMode(uint16_t value);
    // void setMachMode(uint16_t value);
    // void clearOrReset(bool enabled);
};
