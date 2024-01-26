/**
 * KAV A3XX FCU LCD v2.0
 * Written by: James Kavanagh & Keith Greenwood
 * Additional contributions from github.com/elral
 * This library has been written to drive the 'Kav Simulations' FCU LCD Display.
 */
#include "KAV_A3XX_FCU_LCD.h"

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

// BIT MASK //
uint8_t digitPatternFCU[13] = {
    0b11111010, // 0
    0b01100000, // 1
    0b10111100, // 2
    0b11110100, // 3
    0b01100110, // 4
    0b11010110, // 5
    0b11011110, // 6
    0b01110000, // 7
    0b11111110, // 8
    0b11110110, // 9
    0b00000100, // -
    0b00000000, // blank
    0b11001100, // small 0 (For V/S)
};

// PRIVATE METHODS //
void KAV_A3XX_FCU_LCD::displayDigit(uint8_t address, uint8_t digit)
{
    // This ensures that anything over 12 is turned to 'blank', and as it's unsigned, anything less than 0 will become 255, and therefore, 'blank'.
    if (digit > 12) digit = 11;
    buffer[address] = (buffer[address] & 1) | digitPatternFCU[digit];
    refreshLCD(address);
}

void KAV_A3XX_FCU_LCD::refreshLCD(uint8_t address)
{
    ht.write(address * 2, buffer[address], 8);
}

// PUBLIC METHODS //
// Constructor and Setup
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
    // Set the initial labels
    setStartLabels(true);
}

void KAV_A3XX_FCU_LCD::clearLCD()
{
    for (uint8_t i = 0; i < ht.MAX_ADDR; i++)
        ht.write(i, 0);
    memset(buffer, 0, BUFFER_SIZE_MAX);
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

// DISPLAY FUNCTIONS //
void KAV_A3XX_FCU_LCD::setStartLabels(bool enabled)
{
    setAltLabel(enabled);
    setLvlChLabel(enabled);
    setLatLabel(enabled);
}

void KAV_A3XX_FCU_LCD::setTrkMode()
{
    // Set TRK label over heading
    setTrkLabel(true);
    setHdgLabel(false);
    // Set TRK/FPA label in middle
    setTrkFpaLabel(true);
    setHdgVsLabel(false);
    // Set FPA label over vertical speed
    setFpaLabel(true);
    setVsLabel(false);
}

void KAV_A3XX_FCU_LCD::setHdgMode()
{
    // Set HDG label over heading
    setHdgLabel(true);
    setTrkLabel(false);
    // Set HDG / V/S label in middle
    setHdgVsLabel(true);
    setTrkFpaLabel(false);
    // Set V/S label over vertical speed
    setVsLabel(true);
    setFpaLabel(false);
}

// ------------------------ //
// Speed and Mach Functions //
// ------------------------ //
void KAV_A3XX_FCU_LCD::setSpdLabel(bool enabled)
{
    setMachLabel(false);
    SET_BUFF_BIT(SPECIALS, 7, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setMachLabel(bool enabled)
{
    setSpdLabel(false);
    SET_BUFF_BIT(SPECIALS, 6, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setSpdValueInt(uint16_t value)
{
    if (value > 999) value = 999;
    displayDigit(SPD_UNIT, (value % 10));
    value = value / 10;
    displayDigit(SPD_TEN, (value % 10));
    displayDigit(SPD_HUN, (value / 10));
}

void KAV_A3XX_FCU_LCD::setSpdValueFloat(float value)
{
    if (value > 9.99) value = 9.99;
    if (value < 0) value = 0;

    uint16_t intPart = (uint16_t)value;
    uint16_t decPart = (uint16_t)(round((value - intPart) * 100));

    displayDigit(SPD_UNIT, (decPart % 10));
    displayDigit(SPD_TEN, (decPart / 10));
    displayDigit(SPD_HUN, intPart);
}

void KAV_A3XX_FCU_LCD::setSpdDot(bool enabled)
{
    SET_BUFF_BIT(HDG_HUN, 0, enabled);
    refreshLCD(HDG_HUN);
}

void KAV_A3XX_FCU_LCD::setSpdDashes(bool enabled)
{
    uint8_t val;
    if (enabled)
        val = 10; // Choose the '-' symbol
    else
        val = 11; // Choose a blank symbol
    displayDigit(SPD_HUN, val);
    displayDigit(SPD_TEN, val);
    displayDigit(SPD_UNIT, val);

    // // Not sure if this is needed as it likely gets cleared by the above.
    // SET_BUFF_BIT(SPD_TEN, 0, false); // Clear Mach Decimal-point
    // refreshLCD(SPD_TEN);
}

// --------------------------- //
// Heading and Track Functions //
// --------------------------- //
void KAV_A3XX_FCU_LCD::setLatLabel(bool enabled)
{
    SET_BUFF_BIT(HDG_TEN, 0, enabled);
    refreshLCD(HDG_TEN);
}
void KAV_A3XX_FCU_LCD::setHdgLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 5, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setTrkLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 4, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setHdgValue(uint16_t value)
{
    if (value > 999) value = 999;
    displayDigit(HDG_UNIT, (value % 10));
    value = value / 10;
    displayDigit(HDG_TEN, (value % 10));
    displayDigit(HDG_HUN, (value / 10));
}

void KAV_A3XX_FCU_LCD::setHdgDot(bool enabled)
{
    SET_BUFF_BIT(HDG_UNIT, 0, enabled);
    refreshLCD(HDG_UNIT);
}

void KAV_A3XX_FCU_LCD::setHdgDashes(bool enabled)
{
    uint8_t val;
    if (enabled)
        val = 10;
    else
        val = 11;
    displayDigit(HDG_HUN, val);
    displayDigit(HDG_TEN, val);
    displayDigit(HDG_UNIT, val);
}

// ------------------------ //
// Centre Display Functions //
// ------------------------ //
void KAV_A3XX_FCU_LCD::setHdgVsLabel(bool enabled)
{
    SET_BUFF_BIT(ALT_TEN, 0, enabled);
    SET_BUFF_BIT(ALT_THO, 0, enabled);
    refreshLCD(ALT_TEN);
    refreshLCD(ALT_THO);
}

void KAV_A3XX_FCU_LCD::setTrkFpaLabel(bool enabled)
{
    SET_BUFF_BIT(ALT_HUN, 0, enabled);
    SET_BUFF_BIT(ALT_TTH, 0, enabled);
    refreshLCD(ALT_HUN);
    refreshLCD(ALT_TTH);
}

// ------------------ //
// Altitude Functions //
// ------------------ //
void KAV_A3XX_FCU_LCD::setLvlChLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 1, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setAltLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 0, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setAltValue(uint32_t value)
{
    if (value > 99999) value = 99999;
    displayDigit(ALT_UNIT, (value % 10));
    value = value / 10;
    displayDigit(ALT_TEN, (value % 10));
    value = value / 10;
    displayDigit(ALT_HUN, (value % 10));
    value = value / 10;
    displayDigit(ALT_THO, (value % 10));
    displayDigit(ALT_TTH, (value / 10));
}

void KAV_A3XX_FCU_LCD::setAltDot(bool enabled)
{
    SET_BUFF_BIT(VRT_THO, 0, enabled);
    refreshLCD(VRT_THO);
}

void KAV_A3XX_FCU_LCD::setAltDashes(bool enabled)
{
    uint8_t val;
    if (enabled)
        val = 10;
    else
        val = 11;
    displayDigit(ALT_TTH, val);
    displayDigit(ALT_THO, val);
    displayDigit(ALT_HUN, val);
    displayDigit(ALT_TEN, val);
    displayDigit(ALT_UNIT, val);
}


// ------------------------ //
// Vertical Speed Functions //
// ------------------------ //
void KAV_A3XX_FCU_LCD::setVsLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 2, enabled);
    refreshLCD(SPECIALS);
}

void KAV_A3XX_FCU_LCD::setFpaLabel(bool enabled)
{
    SET_BUFF_BIT(SPECIALS, 3, enabled);
    refreshLCD(SPECIALS);
}

// void KAV_A3XX_FCU_LCD::showVerticalValue(int16_t value)
// {
//     if (value > 9999) value = 9999;
//     if (value < -9999) value = -9999;
//     if (value < 0) {
//         // Handle Negative Value
//         value = -value;
//         SET_BUFF_BIT(VRT_TEN, 0, false);
//         SET_BUFF_BIT(VRT_UNIT, 0, true); // Negative sign
//         SET_BUFF_BIT(VRT_HUN, 0, false); // Remove decimal point
//     } else if (value == 0) {
//         // Handle Zero Value
//         SET_BUFF_BIT(VRT_TEN, 0, false);
//         SET_BUFF_BIT(VRT_UNIT, 0, false);
//         SET_BUFF_BIT(VRT_HUN, 0, false); // Remove decimal point
//     } else {
//         // Handle Positive Value
//         SET_BUFF_BIT(VRT_TEN, 0, true); // Positive sign +
//         SET_BUFF_BIT(VRT_UNIT, 0, true); // Negative sign
//         SET_BUFF_BIT(VRT_HUN, 0, false); // Remove decimal point
//     }

//     displayDigit(VRT_UNIT, 12);
//     value = value / 10;
//     displayDigit(VRT_TEN, 12);
//     value = value / 10;
//     displayDigit(VRT_HUN, (value % 10));
//     displayDigit(VRT_THO, (value / 10));
// }
// void KAV_A3XX_FCU_LCD::showFPAValue(int8_t value)
// {
//     if (value > 99) value = 99;
//     if (value < -99) value = -99;
//     if (value < 0) {
//         value = -value;
//         SET_BUFF_BIT(VRT_TEN, 0, false);
//         SET_BUFF_BIT(VRT_UNIT, 0, true);
//         SET_BUFF_BIT(VRT_HUN, 0, true);
//     } else {
//         SET_BUFF_BIT(VRT_TEN, 0, true);
//         SET_BUFF_BIT(VRT_UNIT, 0, true);
//         SET_BUFF_BIT(VRT_HUN, 0, true);
//     }

//     displayDigit(VRT_HUN, (value % 10));
//     displayDigit(VRT_THO, (value / 10));
//     SET_BUFF_BITS(VRT_TEN, 0b11111110, 0);
//     SET_BUFF_BITS(VRT_UNIT, 0b11111110, 0);
//     refreshLCD(VRT_TEN);
//     refreshLCD(VRT_UNIT);
// }
void KAV_A3XX_FCU_LCD::setVsValue(int16_t value)
{
    if (value < 0) value = -value; // Turn a negative into a positive
    if (value > 9999) value = 9999;

    displayDigit(VRT_UNIT, 12);
    value = value / 10;
    displayDigit(VRT_TEN, 12);
    value = value / 10;
    displayDigit(VRT_HUN, (value % 10));
    displayDigit(VRT_THO, (value / 10));

    // Remove decimal point
    SET_BUFF_BIT(VRT_HUN, 0, false);
    refreshLCD(VRT_HUN);
}
void KAV_A3XX_FCU_LCD::setFpaValueInt(int16_t value)
{
    if (value < 0) value = -value; // Turn a negative into a positive
    if (value > 99) value = 99;

    displayDigit(VRT_HUN, (value % 10));
    displayDigit(VRT_THO, (value / 10));
    SET_BUFF_BITS(VRT_TEN, 0b11111110, 0);  // Turn off the last 2 chars
    SET_BUFF_BITS(VRT_UNIT, 0b11111110, 0); // Turn off the last 2 chars
    refreshLCD(VRT_TEN);
    refreshLCD(VRT_UNIT);

    // Add decimal point
    SET_BUFF_BIT(VRT_HUN, 0, true);
    refreshLCD(VRT_HUN);
}

void KAV_A3XX_FCU_LCD::setFpaValueFloat(float value)
{
    if (value < 0) value = -value; // Turn a negative into a positive
    if (value > 9.9) value = 9.9;

    uint16_t intPart = (uint16_t)value;
    uint16_t decPart = (uint16_t)(round((value - intPart) * 100));

    displayDigit(VRT_HUN, (decPart / 10));
    displayDigit(VRT_THO, intPart);
    SET_BUFF_BITS(VRT_TEN, 0b11111110, 0);  // Turn off the last 2 chars
    SET_BUFF_BITS(VRT_UNIT, 0b11111110, 0); // Turn off the last 2 chars
    refreshLCD(VRT_TEN);
    refreshLCD(VRT_UNIT);

    // Add decimal point
    SET_BUFF_BIT(VRT_HUN, 0, true);
    refreshLCD(VRT_HUN);
}

void KAV_A3XX_FCU_LCD::setPlus(bool enabled)
{
    if (enabled) {
        SET_BUFF_BIT(VRT_TEN, 0, true); // Positive sign +
        SET_BUFF_BIT(VRT_UNIT, 0, true); // Negative sign
        
    } else {
        // Remove the plus sign
        SET_BUFF_BIT(VRT_TEN, 0, false);
        SET_BUFF_BIT(VRT_UNIT, 0, false);
    }
    refreshLCD(VRT_TEN);
    refreshLCD(VRT_UNIT);
}

void KAV_A3XX_FCU_LCD::setMinus(bool enabled)
{
    if (enabled) {
        SET_BUFF_BIT(VRT_TEN, 0, false);
        SET_BUFF_BIT(VRT_UNIT, 0, true); // Negative sign
    } else {
        // Remove the minus sign
        SET_BUFF_BIT(VRT_TEN, 0, false);
        SET_BUFF_BIT(VRT_UNIT, 0, false);
    }
    refreshLCD(VRT_TEN);
    refreshLCD(VRT_UNIT);
}

void KAV_A3XX_FCU_LCD::setVsDashes(bool enabled)
{
    uint8_t val;
    if (enabled) {
        val = 10;
        SET_BUFF_BIT(VRT_TEN, 0, false); // Remove the plus segment
        SET_BUFF_BIT(VRT_UNIT, 0, true); // Set the plus/minus to minus
        SET_BUFF_BIT(VRT_HUN, 0, false); // Remove the decimal point
    } else {
        val = 11;
        SET_BUFF_BIT(VRT_UNIT, 0, false); // Turn it off
    }
    displayDigit(VRT_THO, val);
    displayDigit(VRT_HUN, val);
    displayDigit(VRT_TEN, val);
    displayDigit(VRT_UNIT, val);
}

// ----------------------------- //
// Preset and Combined Functions //
// ----------------------------- //

/**
  * @brief  Handles a SPD, MACH or Dashes message.
  * It must take only intergers, and will handle the decimal point itself.
  * I.e., if you want to display 1.5, you must pass 15. 
  * Or if you want to display 0.5, you must pass 5.
  * 
  * @param  data: The data to display.
  */
void KAV_A3XX_FCU_LCD::showSPDInt(char* data)
{
    // Check to see if it's dashes first
    if (data[0] == '-') {
        setSpdDashes(true);
        return;
    }    
    // Else it's a number
    uint16_t value = strtoul(data, NULL, 10);
    setSpdValueInt(value);
    if (value < 100) {
        SET_BUFF_BIT(SPD_TEN, 0, true);  // Set Mach Decimal-point
        setMachLabel(true);
    } else {
        SET_BUFF_BIT(SPD_TEN, 0, false); // Clear Mach Decimal-point
        setSpdLabel(true);
    }
    refreshLCD(SPD_TEN);
}

/**
  * @brief  Handles a SPD, MACH or Dashes message.
  * It can take floats, and will handle the decimal point itself.
  * 
  * @param  data: The data to display.
  */
void KAV_A3XX_FCU_LCD::showSPDFloat(char* data)
{
    // Check to see if it's dashes first
    if (data[0] == '-') {
        setSpdDashes(true);
        return;
    }    
    // Is it a float
    if (data[1] == '.') {
        float value = strtod(data, NULL);
        setSpdValueFloat(value);
        if (value < 9.9) {
            SET_BUFF_BIT(SPD_TEN, 0, true);  // Set Mach Decimal-point
            setMachLabel(true);
        } else {
            SET_BUFF_BIT(SPD_TEN, 0, false); // Clear Mach Decimal-point
            setSpdLabel(true);
        }
        refreshLCD(SPD_TEN);
    } else {
        uint16_t value = strtoul(data, NULL, 10);
        setSpdValueInt(value);
        if (value < 999) {
            SET_BUFF_BIT(SPD_TEN, 0, true);  // Set Mach Decimal-point
            setMachLabel(true);
        } else {
            SET_BUFF_BIT(SPD_TEN, 0, false); // Clear Mach Decimal-point
            setSpdLabel(true);
        }
        refreshLCD(SPD_TEN);
    }
}

// void KAV_A3XX_FCU_LCD::showHDG(uint16_t value)
// {
//     setHdgValue(value);
// }

// void KAV_A3XX_FCU_LCD::showALT(uint32_t value)
// {
//     setAltValue(value);
// }

void KAV_A3XX_FCU_LCD::showVSInt(char* data)
{
    // TODO: Implement V/S as interger, similar to `showSPDInt`
    // Display only the value, ignore + or - sign
}
void KAV_A3XX_FCU_LCD::showVSFloat(char* data)
{
    // TODO: Implement V/S as float, similar to `showSPDFloat`
    // Display only the value, ignore + or - sign
}
void KAV_A3XX_FCU_LCD::showVSInt_PlusMinus(char* data)
{
    // TODO: Implement V/S as interger, similar to `showSPDInt`
    // Display the value and handle the + or - sign
    // We can use the `setPlus` and `setMinus` functions
}
void KAV_A3XX_FCU_LCD::showVSFloat_PlusMinus(char* data)
{
    // TODO: Implement V/S as float, similar to `showSPDFloat`
    // Display the value and handle the + or - sign
    // We can use the `setPlus` and `setMinus` functions
}

void KAV_A3XX_FCU_LCD::toggleTrkHdgMode(bool enabled)
{
    if (enabled)
        setTrkMode();
    else
        setHdgMode();        
}

// -------------------------------------- //
// SET Function - For Use with MobiFlight //
// -------------------------------------- //
void KAV_A3XX_FCU_LCD::set(int16_t messageID, char *setPoint)
{
    // int32_t data = strtoul(setPoint, NULL, 10);
    int32_t data = strtol(setPoint, NULL, 10); // Keep the sign for some values?

    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -1 will be send from the connector when Mobiflight is closed
        Put in your code to shut down your custom device (e.g. clear a display)
        MessageID == -2 will be send from the connector when PowerSavingMode is entered
        Put in your code to enter this mode (e.g. clear a display)
    ********************************************************************************** */
    switch(messageID)
    {
        case -1:
            break; // Ignore for now, handle this condition later.
        case -2:
            break; // Ignore for now, handle this condition later.
        case 0:
            // TODO: Implement full switch case
        default:
            break;
    }

    // if (messageID == -1)
    //     return; // Ignore for now, handle this condition later.
    // else if (messageID == -2)
    //     return; // Ignore for now, handle this condition later.
    // else if (messageID == 0)
    //     setSpeedMode((uint16_t)data);
    // else if (messageID == 1)
    //     setMachMode((uint16_t)data);
    // else if (messageID == 2)
    //     showHeadingValue((uint16_t)data);
    // else if (messageID == 3)
    //     showAltitudeValue((uint32_t)data);
    // else if (messageID == 4)
    //     showVerticalValue((int16_t)data);
    // else if (messageID == 5)
    //     showFPAValue((int8_t)data);
    // else if (messageID == 6)
    //     setSpeedDashes((int8_t)data);
    // else if (messageID == 7)
    //     setHeadingDashes((int8_t)data);
    // else if (messageID == 8)
    //     setAltitudeDashes((int8_t)data);
    // else if (messageID == 9)
    //     setVrtSpdDashes((int8_t)data);
    // else if (messageID == 10)
    //     setSpeedDot((int8_t)data);
    // else if (messageID == 11)
    //     setHeadingDot((int8_t)data);
    // else if (messageID == 12)
    //     setAltitudeDot((int8_t)data);
    // else if (messageID == 13)
    //     toggleTrkHdgMode((int8_t)data);
    // else if (messageID == 14)
    //     setSpeedLabel((int8_t)data);
    // else if (messageID == 15)
    //     setMachLabel((int8_t)data);
    // else if (messageID == 16)
    //     showSpeedValue((uint16_t)data);
    // else if (messageID == 17)
    //     clearOrReset((int8_t)data);
}
