#include "KAV_A3XX_RUDDER_LCD.h"
#include "KAV_GetDigitPattern.h"

#define DIGIT_LR 0
#define DIGIT_TWO 1
#define DIGIT_THREE 2
#define DIGIT_FOUR 3

// Helper macro to set a specific bit in the buffer
#define SET_BUFF_BIT(addr, bit, enabled) buffer[addr] = (buffer[addr] & (~(1 << (bit)))) | (((enabled & 1)) << (bit));

/**
 * Setup the LCD
 * This function is called when the deivce is initialised using the 'attach' function.
 * It sets up the LCD and clears it.
 */
void KAV_A3XX_RUDDER_LCD::begin()
{
    ht_rudder.begin();
    ht_rudder.sendCommand(HT1621::RC256K);
    ht_rudder.sendCommand(HT1621::BIAS_THIRD_4_COM);
    ht_rudder.sendCommand(HT1621::SYS_EN);
    ht_rudder.sendCommand(HT1621::LCD_ON);
    // This clears the LCD
    for (uint8_t i = 0; i < ht_rudder.MAX_ADDR; i++)
        ht_rudder.write(i, 0);

    // Initialises the buffer to all 0's.
    memset(buffer, 0, BUFFER_SIZE_MAX);
}

/**
 * Initialise the LCD (Entry Point)
 * This function is called to initialise the LCD.
 * @param CS The Chip Select pin
 * @param CLK The Clock pin
 * @param DATA The Data pin
 */
void KAV_A3XX_RUDDER_LCD::attach(byte CS, byte CLK, byte DATA)
{
    _CS = CS;
    _CLK = CLK;
    _DATA = DATA;
    _initialised = true;
    begin();
}

/**
 * Detach the LCD
 * Required for MobiFLight
 */
void KAV_A3XX_RUDDER_LCD::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

/**
 * Refresh the LCD
 * After a change is made to a segment, the display must be refreshed so that
 * the change is visible.
 * @param address The address to refresh
 */
void KAV_A3XX_RUDDER_LCD::refreshLCD(uint8_t address)
{
    ht_rudder.write(address * 2, buffer[address], 8);
}

/**
 * Refresh the LCD
 * After a change is made to a segment, the display must be refreshed so that
 * the change is visible.
 * @param address The address to refresh
 * @param digits The number of digits to refresh
 */
void KAV_A3XX_RUDDER_LCD::refreshLCD(uint8_t address, uint8_t digits)
{
    for (uint8_t i = 0; i < digits; i++) {
        refreshLCD(address + i);
    }
}

/**
 * Clear the LCD
 * This function clears the LCD and resets the buffer.
 */
void KAV_A3XX_RUDDER_LCD::clearLCD()
{
    for (uint8_t i = 0; i < ht_rudder.MAX_ADDR; i++)
        ht_rudder.write(i, 0);
    memset(buffer, 0, BUFFER_SIZE_MAX);
}

void KAV_A3XX_RUDDER_LCD::clearDigit(uint8_t address)
{
    ht_rudder.write(address * 2, 0);
    buffer[address] = 0;
}

// 'L', 'R' and Dot Functions
/**
 * Set the first character to show 'L' or blank.
 * @param enabled Whether to show 'L' (true) or blank (false)
 */
void KAV_A3XX_RUDDER_LCD::setLeft(bool enabled)
{
    if (enabled) {
        getDigitPattern(buffer, DIGIT_LR, (char*)"L", 1);
    } else {
        // enable also clearing the decimal point if 'R' was displayed before
        getDigitPattern(buffer, DIGIT_LR, (char*)" ", 1,(1<<0));
    }
    refreshLCD(DIGIT_LR);
}

/**
 * Set the first character to show 'R' or blank.
 * @param enabled Whether to show 'R' (true) or blank (false)
 */
void KAV_A3XX_RUDDER_LCD::setRight(bool enabled)
{
    if (enabled) {
        // 'R' is not definded in char table, use 'A' and decimal point to set required segements
        getDigitPattern(buffer, DIGIT_LR, (char*)"A.", 2, (1<<0));
    } else {
        getDigitPattern(buffer, DIGIT_LR, (char*)" ", 1,(1<<0));
    }
    refreshLCD(DIGIT_LR);
}

/**
 * Set the dot to show or hide.
 * @param enabled Whether to show the dot (true) or hide it (false)
 */
void KAV_A3XX_RUDDER_LCD::setDot(bool enabled)
{
    // The dot is the 4th bit of the DIGIT_THREE address.
    SET_BUFF_BIT(DIGIT_THREE, 4, enabled);
    refreshLCD(DIGIT_THREE);
}

/**
 * Set the value of the LCD using an integer.
 * 'L' or 'R' must be set AFTER this function call
 * @param value The value to display
 */
void KAV_A3XX_RUDDER_LCD::setValue(int16_t value)
{
    setValue((float)value/10);
}

/**
 * Set the value of the LCD using an integer.
 * 'L' or 'R' must be set AFTER this function call
 * @param value The value to display
 */
void KAV_A3XX_RUDDER_LCD::setValue(float value)
{
    char bufferDigits[10] = {0};
    // Convert the number from negative to positive if it's negative.
    if (value < 0) {
        value = -value;
    }
    dtostrf(value, 4, 1, bufferDigits);
    showLandRValue(bufferDigits);
}

// Show values as a combined function
/**
 * Show the value on the display with the 'L' character enabled using an integer.
 * @param value The value to display
 */
void KAV_A3XX_RUDDER_LCD::showLeftValue(uint16_t value)
{
    showLandRValue((float)value / -10);
}

/**
 * Show the value on the display with the 'R' character enabled using an integer.
 * @param value The value to display
 */
void KAV_A3XX_RUDDER_LCD::showRightValue(uint16_t value)
{
    showLandRValue((float)value/10);
}

/**
 * Show the value on the display with the 'L' or 'R' character enabled using an integer.
 * @param value The value to display
*/
void KAV_A3XX_RUDDER_LCD::showLandRValue(int16_t value)
{
    showLandRValue((float)value/10);
}

/**
 * Show the value on the display with the 'L' or 'R' character enabled using an integer.
 * @param value The value to display
*/
void KAV_A3XX_RUDDER_LCD::showLandRValue(float value)
{
    char bufferDigits[10] = {0};
    if (value < 0)
    {
        setLeft(true);
        value = -value;
        bufferDigits[0] = 'L';
    }
    else
    {
        setRight(true);
        bufferDigits[0] = 'R';
    }
    dtostrf(value, 4, 1, &bufferDigits[1]);
    showLandRValue(bufferDigits);
}

/**
 * Show the value on the display, checks for 'L' or 'R' character.
 * @param value The value to display
*/
void KAV_A3XX_RUDDER_LCD::showLandRValue(char *value)
{
    if (value[0] == 'L')
    {
        setLeft(true);
        getDigitPattern(buffer, DIGIT_TWO, &value[1], 3, (1<<1));
    }
    else if (value[0] == 'R')
    {
        setRight(true);
        getDigitPattern(buffer, DIGIT_TWO, &value[1], 3, (1<<1));
    }
    else {
        setLeft(false);
        setRight(false);
        getDigitPattern(buffer, DIGIT_TWO, value, 3, (1<<1));
    }
    refreshLCD(DIGIT_LR, 4);
    strncpy(_lastRudderValue, value, sizeof(_lastRudderValue));
}

void KAV_A3XX_RUDDER_LCD::setPowerSave(bool enabled) 
{
    if (enabled) {
        clearLCD();
    } else {
        showLandRValue(_lastRudderValue);
    }
}

// Global Functions

void KAV_A3XX_RUDDER_LCD::setAnnunciatorTest(bool enabled)
{
    if (enabled) {
        for (uint8_t i = 0; i < ht_rudder.MAX_ADDR; i++)
            ht_rudder.write(i, 0xFF);
    } else {
        setPowerSave(false);
    }
}

/**
 * Handle MobiFlight Commands
 * This function shouldn't be called be a user, it should only be called by the
 * custom device function. This is where data from MobiFlight enters the
 * library and is handled to be displayed on the LCD.
 * @param cmd The command from MobiFlight
 * @see handleMobiFlightRaw
 */
void KAV_A3XX_RUDDER_LCD::set(int16_t messageID, char *setPoint)
{
    // Using `strtol()` instead of `atoi()` to allow for larger value numbers.
    int32_t data = strtol(setPoint, NULL, 10);
    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -2 will be send from the board when PowerSavingMode is set
            Message will be "0" for leaving and "1" for entering PowerSavingMode
        MessageID == -1 will be send from the connector when Connector stops running
        Put in your code to enter this mode (e.g. clear a display)
    ********************************************************************************** */
    if (messageID == -1)
        setPowerSave(true);
    else if (messageID == -2)
        setPowerSave((bool)setPoint);
    else if (messageID == 0)
        // setLeft((uint16_t)data); deprecated
        return;
    else if (messageID == 1)
        // setRight((uint16_t)data); deprecated
        return;
    else if (messageID == 2)
        // setDot((uint16_t)data); deprecated
        return;
    else if (messageID == 3)
        // This one needs to keep it's sign, so using `int16_t`.
        // setValue((int16_t)data); deprecated
        return;
    else if (messageID == 4)
        showLeftValue((uint16_t)data);
    else if (messageID == 5)
        // showRightValue((uint16_t)data); deprecated
        return;
    else if (messageID == 6)
        // This one needs to keep it's sign, so using `int16_t`.
        // showLandRValue((int16_t)data); deprecated
        return;
    else if (messageID == 7)
        setAnnunciatorTest((bool)data);
    else if (messageID == 8)
        // This one shows the string and checks for 'L' or 'R' as first character
        showLandRValue(setPoint);
    else if (messageID == 9)
        setPowerSave((bool)setPoint);
}
