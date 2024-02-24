/**
 * KAV A3XX FCU LCD
 * Written by: James Kavanagh, Ralf (@elral), & Keith Greenwood
 * This library has been written to drive the 'Kav Simulations' FCU LCD Display.
 */

#include "KAV_A3XX_FCU_LCD.h"
#include "KAV_GetDigitPattern.h"


#define SPD_HUN  0
#define SPD_TEN  1
#define SPD_UNIT 2
#define HDG_HUN  3
#define HDG_TEN  4
#define HDG_UNIT 5
#define ALT_TTH  6
#define ALT_THO  7
#define ALT_HUN  8
#define ALT_TEN  9
#define ALT_UNIT 10
#define VRT_THO  11
#define VRT_HUN  12
#define VRT_TEN  13
#define VRT_UNIT 14
#define SPECIALS 15

#define SET_BUFF_BITS(addr, bitMask, enabledMask) buffer[addr] = (buffer[addr] & (~(bitMask))) | (enabledMask)
#define SET_BUFF_BIT(addr, bit, enabled)          buffer[addr] = (buffer[addr] & (~(1 << (bit)))) | (((enabled & 1)) << (bit))

void KAV_A3XX_FCU_LCD::begin()
{
    ht.begin();
    ht.sendCommand(HT1621::RC256K);
    ht.sendCommand(HT1621::BIAS_THIRD_4_COM);
    ht.sendCommand(HT1621::SYS_EN);
    ht.sendCommand(HT1621::LCD_ON);
    // This clears the LCD
    for (uint8_t i = 0; i < ht.MAX_ADDR; i++)
        ht.write(i, 0);

    // Initialises the buffer to all 0's.
    memset(buffer, 0, BUFFER_SIZE_MAX);
    setStartLabels();
}

void KAV_A3XX_FCU_LCD::attach(byte CS, byte CLK, byte DATA)
{
    _CS          = CS;
    _CLK         = CLK;
    _DATA        = DATA;
    _initialised = true;
    begin();
}
void KAV_A3XX_FCU_LCD::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

void KAV_A3XX_FCU_LCD::refreshLCD(uint8_t address)
{
    ht.write(address * 2, buffer[address], 8);
}

void KAV_A3XX_FCU_LCD::refreshLCD(uint8_t address, uint8_t digits)
{
    for (uint8_t i = 0; i < digits; i++) {
        refreshLCD(address + i);
    }
}

void KAV_A3XX_FCU_LCD::clearLCD()
{
    for (uint8_t i = 0; i < ht.MAX_ADDR; i++)
        ht.write(i, 0);
    memset(buffer, 0, BUFFER_SIZE_MAX);
}

// Speed
void KAV_A3XX_FCU_LCD::setSpeedLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 7, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setMachLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 6, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setSpeedDot(bool enabled)
{
    SET_BUFF_BIT(HDG_HUN, 0, enabled);
    refreshLCD(HDG_HUN);
}

//
// With setting labels
//

// value 0 to 999
// no dot is shown
void KAV_A3XX_FCU_LCD::setSpeedMode(uint16_t value)
{
    if (value > 999) value = 999;
    setSpeedLabel(true);
    setMachLabel(false);
    showSpeedMachValue((float)value);
}

// value 0 to 999
// value = 42 will be shown as 0.42
void KAV_A3XX_FCU_LCD::setMachMode(uint16_t value)
{
    setSpeedLabel(false);
    setMachLabel(true);
    showSpeedMachValue((float)value/100);
}

// value 0 to 0.99
// value = 0.42 will be shown as 0.42
void KAV_A3XX_FCU_LCD::setMachMode(float value)
{
    setSpeedLabel(false);
    setMachLabel(true);
    showSpeedMachValue(value);
}

//
// Without setting labels
//

// value 0 to 999
// no dot is shown
void KAV_A3XX_FCU_LCD::showSpeedValue(uint16_t value)
{
    if (value > 999) value = 999;
    showSpeedMachValue((float)value);
}

// value 0 to 999
// value = 42 will be shown as 0.42
void KAV_A3XX_FCU_LCD::showMachValue(uint16_t value)
{
    if (value > 999) value = 999;
    showSpeedMachValue((float)value/100);
}

// value 0 to 999 for Speed
// value 0 to 0.99 for Mach
// dot will be set accordingly
void KAV_A3XX_FCU_LCD::showSpeedMachValue(float value)
{
    char bufferDigits[10] = {0};
    if (value < 1)
        dtostrf(value, 4,2, bufferDigits);
    else
        snprintf(bufferDigits, 10, "%03d", (uint16_t)value);
    showSpeedMachValue(bufferDigits);
}

// string will be displayed, build him as required
void KAV_A3XX_FCU_LCD::showSpeedMachValue(char* data)
{
    buffer[SPD_TEN] &= 0xFE;
    buffer[SPD_UNIT] &= 0xFE;
    getFCUDigitPattern(buffer, SPD_HUN, data, 3, (1 << 0));
    refreshLCD(SPD_HUN, 3);
}

// Heading
void KAV_A3XX_FCU_LCD::setHeadingLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 5, enabled);
    SET_BUFF_BIT(ALT_TEN, 0, enabled);
    refreshLCD(SPECIALS);
    refreshLCD(ALT_TEN);
}

void KAV_A3XX_FCU_LCD::setTrackLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 4, enabled);
    SET_BUFF_BIT(ALT_HUN, 0, enabled);
    refreshLCD(SPECIALS);
    refreshLCD(ALT_HUN);
}

void KAV_A3XX_FCU_LCD::setLatitudeLabel(bool enabled)
{
    SET_BUFF_BIT(HDG_TEN, 0, enabled);
    refreshLCD(HDG_TEN);
}

void KAV_A3XX_FCU_LCD::setHeadingDot(bool enabled)
{
    SET_BUFF_BIT(HDG_UNIT, 0, enabled);
    refreshLCD(HDG_UNIT);
}

void KAV_A3XX_FCU_LCD::showHeadingValue(uint16_t value)
{
    char bufferDigits[10] = {0};
    
    if (value > 999) value = 999;
    snprintf(bufferDigits, 10, "%03d", (uint16_t)value);
    showHeadingValue(bufferDigits);
}

void KAV_A3XX_FCU_LCD::showHeadingValue(char* data)
{
    getFCUDigitPattern(buffer, HDG_HUN, data, 3);
    refreshLCD(HDG_HUN, 3);
}

// Altitude
void KAV_A3XX_FCU_LCD::setAltitudeLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 0, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setLvlChLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 1, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setAltitudeDot(bool enabled)
{
    SET_BUFF_BIT(VRT_THO, 0, enabled);
    refreshLCD(VRT_THO);
}

void KAV_A3XX_FCU_LCD::showAltitudeValue(uint32_t value)
{
    char bufferDigits[10] = {0};

    snprintf(bufferDigits, 10, "%05u", (uint16_t)value);
    showAltitudeValue(bufferDigits);
}

void KAV_A3XX_FCU_LCD::showAltitudeValue(char* data)
{
    getFCUDigitPattern(buffer, ALT_TTH, data, 5);
    refreshLCD(ALT_TTH, 5);

}

// Vertical
void KAV_A3XX_FCU_LCD::setVrtSpdLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 2, enabled);
    SET_BUFF_BIT(ALT_THO, 0, enabled);
    refreshLCD(SPECIALS);
    refreshLCD(ALT_THO);
}

void KAV_A3XX_FCU_LCD::setFPALabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 3, enabled);
    SET_BUFF_BIT(ALT_TTH, 0, enabled);
    refreshLCD(SPECIALS);
    refreshLCD(ALT_TTH);
}

void KAV_A3XX_FCU_LCD::setSignLabel(bool enabled)
{
    vertSignEnabled = enabled;
}

// value 0 to +/-9999 for Vertical value
// 'oo' will be handled inside function
void KAV_A3XX_FCU_LCD::showVerticalValue(int16_t value)
{
    char bufferDigits[10] = {0};

    if (value > 9999) value = 9999;
    if (value < -9999) value = -9999;
    if (value < 0) {
        bufferDigits[0] = '-';
    } else {
        bufferDigits[0] = '+';
    }
    snprintf(&bufferDigits[1], 10, "%02doo", abs(value)/100);
    showVerticalFPAValue(bufferDigits);
}

// value 0 to +/-99 for FPA value
// 99 will be displaye as 9.9
void KAV_A3XX_FCU_LCD::showFPAValue(int8_t value)
{
    showFPAValue((float)value/10);
}

// value 0 to +/-9.9 for FPA value
void KAV_A3XX_FCU_LCD::showFPAValue(float value)
{
    char bufferDigits[10] = {0};

    if (value >= 10) value = 9.9;
    if (value <= -10) value = -9.9;

    if (value < 0) {
        bufferDigits[0] = '-';
        value = -value;
    } else {
        bufferDigits[0] = '+';
    }
    dtostrf(value, 3,1, &bufferDigits[1]);
    bufferDigits[4] = ' ';
    bufferDigits[5] = ' ';
    showVerticalFPAValue(bufferDigits);
}

// String will be displayed as it is
void KAV_A3XX_FCU_LCD::showVerticalFPAValue(char* data)
{
    uint8_t dpMask = (1 << 0);
    if (data[0] == '-') {
        SET_BUFF_BIT(VRT_TEN, 0, false);
        SET_BUFF_BIT(VRT_UNIT, 0, vertSignEnabled);
        SET_BUFF_BIT(VRT_HUN, 0, false);
        getFCUDigitPattern(buffer, VRT_THO, &data[1], 4, dpMask);
    } else if (data[0] == '+') {
        SET_BUFF_BIT(VRT_TEN, 0, vertSignEnabled);
        SET_BUFF_BIT(VRT_UNIT, 0, vertSignEnabled);
        SET_BUFF_BIT(VRT_HUN, 0, false);
        getFCUDigitPattern(buffer, VRT_THO, &data[1], 4, dpMask);
    } else {
        SET_BUFF_BIT(VRT_TEN, 0, false);
        SET_BUFF_BIT(VRT_UNIT, 0, false);
        SET_BUFF_BIT(VRT_HUN, 0, false);
        getFCUDigitPattern(buffer, VRT_THO, data, 4, dpMask);
    }
    refreshLCD(VRT_THO, 4);
}

// Preset States
void KAV_A3XX_FCU_LCD::setSpeedDashes(bool state)
{
    if (state)
        showSpeedMachValue((char*)"---");
    else
        showSpeedMachValue((char*)"   ");
}

void KAV_A3XX_FCU_LCD::setHeadingDashes(bool state)
{
    if (state)
        showHeadingValue((char*)"---");
    else
        showHeadingValue((char*)"   ");
}

void KAV_A3XX_FCU_LCD::setAltitudeDashes(bool state)
{
    if (state)
        showAltitudeValue((char*)"-----");
    else
        showAltitudeValue((char*)"     ");
}

void KAV_A3XX_FCU_LCD::setVrtSpdDashes(bool state)
{
    if (state)
        showVerticalFPAValue((char*)"-----");
    else
        showVerticalFPAValue((char*)"     ");
}

void KAV_A3XX_FCU_LCD::setStartLabels()
{
    setAltitudeLabel(true);
    setLvlChLabel(true);
    setLatitudeLabel(true);
}

void KAV_A3XX_FCU_LCD::toggleTrkHdgMode(bool state)
{
    if (state)
        setTrackMode();
    else
        setHeadingMode();
}

void KAV_A3XX_FCU_LCD::setHeadingMode()
{
    setHeadingLabel(true);
    setTrackLabel(false);
    setVrtSpdLabel(true);
    setFPALabel(false);
    trkActive = false;
}

void KAV_A3XX_FCU_LCD::setTrackMode()
{
    setHeadingLabel(false);
    setTrackLabel(true);
    setVrtSpdLabel(false);
    setFPALabel(true);
    trkActive = true;
}

void KAV_A3XX_FCU_LCD::toggleSpeedMachMode(bool state) {
    if (state) {
        setMachLabel(true);
        setSpeedLabel(false);
    } else {
        setMachLabel(false);
        setSpeedLabel(true);
    }
}

// Global Functions

void KAV_A3XX_FCU_LCD::clearOrReset(bool enabled)
{
    if (enabled)
        setStartLabels();
    else
        clearLCD();
}

void KAV_A3XX_FCU_LCD::set(int16_t messageID, char *setPoint)
{
    int32_t data = strtol(setPoint, NULL, 10);
    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -1 will be send from the connector when Mobiflight is closed
        Put in your code to shut down your custom device (e.g. clear a display)
        MessageID == -2 will be send from the connector when PowerSavingMode is entered
        Put in your code to enter this mode (e.g. clear a display)
    ********************************************************************************** */
    if (messageID == -1)
        return; // Ignore for now, handle this condition later.
    else if (messageID == -2)
        return; // Ignore for now, handle this condition later.
    else if (messageID == 0)
        setSpeedMode((uint16_t)data);
    else if (messageID == 1)
        setMachMode((uint16_t)data);
    else if (messageID == 2)
        showHeadingValue((uint16_t)data);
    else if (messageID == 3)
        showAltitudeValue((uint32_t)data);
    else if (messageID == 4)
        showVerticalValue((int16_t)data);
    else if (messageID == 5)
        showFPAValue((int8_t)data);
    else if (messageID == 6)
        setSpeedDashes((bool)data);
    else if (messageID == 7)
        setHeadingDashes((bool)data);
    else if (messageID == 8)
        setAltitudeDashes((bool)data);
    else if (messageID == 9)
        setVrtSpdDashes((bool)data);
    else if (messageID == 10)
        setSpeedDot((bool)data);
    else if (messageID == 11)
        setHeadingDot((bool)data);
    else if (messageID == 12)
        setAltitudeDot((bool)data);
    else if (messageID == 13)
        toggleTrkHdgMode((bool)data);
    else if (messageID == 14)
        setSpeedLabel((int8_t)data);
    else if (messageID == 15)
        setMachLabel((int8_t)data);
    else if (messageID == 16)
        showSpeedValue((uint16_t)data);
    else if (messageID == 17)
        clearOrReset((int8_t)data);
    else if (messageID == 18)
        showSpeedMachValue(setPoint);
    else if (messageID == 19)
        showHeadingValue(setPoint);
    else if (messageID == 20)
        showAltitudeValue(setPoint);
    else if (messageID == 21)
        showVerticalFPAValue(setPoint);
    else if (messageID == 22)
        toggleSpeedMachMode((bool)data);
}
