#include "KAV_A3XX_RAD_TCAS_LCD.h"
#include "KAV_GetDigitPattern.h"

#define DIGIT_ONE 0
#define DIGIT_TWO 1
#define DIGIT_THREE 2
#define DIGIT_FOUR 3
#define DIGIT_FIVE 4
#define DIGIT_SIX 5

// Helper macro to set a specific bit in the buffer
#define SET_BUFF_BIT(addr, bit, enabled) buffer[addr] = (buffer[addr] & (~(1 << (bit)))) | (((enabled & 1)) << (bit));

/**
 * Setup the LCD
 * This function is called when the deivce is initialised using the 'attach' function.
 * It sets up the LCD and clears it.
 */
void KAV_A3XX_RAD_TCAS_LCD::begin()
{
    ht_rad_tcas.begin();
    ht_rad_tcas.sendCommand(HT1621::RC256K);
    ht_rad_tcas.sendCommand(HT1621::BIAS_THIRD_4_COM);
    ht_rad_tcas.sendCommand(HT1621::SYS_EN);
    ht_rad_tcas.sendCommand(HT1621::LCD_ON);
    // This clears the LCD
    for (uint8_t i = 0; i < ht_rad_tcas.MAX_ADDR; i++)
        ht_rad_tcas.write(i, 0);

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
void KAV_A3XX_RAD_TCAS_LCD::attach(byte CS, byte CLK, byte DATA)
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
void KAV_A3XX_RAD_TCAS_LCD::detach()
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
void KAV_A3XX_RAD_TCAS_LCD::refreshLCD(uint8_t address)
{
    ht_rad_tcas.write(address * 2, buffer[address], 8);
}

/**
 * Refresh the LCD
 * After a change is made to a segment, the display must be refreshed so that
 * the change is visible.
 * @param address The address to refresh
 * @param digits The number of digits to refresh
 */
void KAV_A3XX_RAD_TCAS_LCD::refreshLCD(uint8_t address, uint8_t digits)
{
    for (uint8_t i = 0; i < digits; i++) {
        refreshLCD(address + i);
    }
}

/**
 * Clear the LCD
 * This function clears the LCD and resets the buffer.
 */
void KAV_A3XX_RAD_TCAS_LCD::clearLCD()
{
    char bufferDigits[12] = {0};
    snprintf(bufferDigits, 12, "      ");
    showValue(bufferDigits);
}

void KAV_A3XX_RAD_TCAS_LCD::clearDigit(uint8_t address)
{
    ht_rad_tcas.write(address * 2, 0);
    buffer[address] = 0;
}

/**
 * Set the dot to show or hide.
 * @param enabled Whether to show the dot (true) or hide it (false)
 */
void KAV_A3XX_RAD_TCAS_LCD::setRadioDot(bool enabled)
{
    // The dot is the 4th bit of the DIGIT_THREE address.
    SET_BUFF_BIT(DIGIT_THREE, 4, enabled);
    refreshLCD(DIGIT_THREE);
}

/**
 * Set a specific dot to show or hide.
 * The address for each character are 0 indexed:
 * 0: DIGIT_ONE, 1: DIGIT_TWO, 2: DIGIT_THREE, 3: DIGIT_FOUR, 4: DIGIT_FIVE, 5: DIGIT_SIX
 * @param address The address of the dot to show or hide
 * @param enabled Whether to show the dot (true) or hide it (false)
 */
void KAV_A3XX_RAD_TCAS_LCD::setSpecificDot(uint8_t address, bool enabled)
{
    // The dot is the 4th bit of the address.
    SET_BUFF_BIT(address, 4, enabled);
    refreshLCD(address);
}

/**
 * Set all dots to show or hide.
 * @param enabled Whether to show the dots (true) or hide them (false)
 */
void KAV_A3XX_RAD_TCAS_LCD::setAllDots(bool enabled)
{
    for (uint8_t i = 0; i < 6; i++)
        setSpecificDot(i, enabled);
}

/**
 * Set the value of the LCD using an integer for radio (6 characters).
 * The dots get NOT cleared. If required use showRadio()
 * @param value The value to display
 */
void KAV_A3XX_RAD_TCAS_LCD::setRadioValue(uint32_t value)
{
    char bufferDigits[12] = {0};
    if (value == 0)
        snprintf(bufferDigits, 12, "000000");
    else
        dtostrf((float)value/1000, 6, 3, bufferDigits);

    getDigitPattern(buffer, DIGIT_ONE, bufferDigits, 6);
    refreshLCD(DIGIT_ONE, 6);
}

/**
 * Set the value of the LCD using an integer for TCAS (4 characters).
 * The dots get NOT cleared. If required use showTcas()
 * @param value The value to display
 */
void KAV_A3XX_RAD_TCAS_LCD::setTcasValue(uint16_t value)
{
    char bufferDigits[12] = {0};
    if (value > 9999)
        value = 9999;
    
    snprintf(bufferDigits, 12, " %04d ", value);
    getDigitPattern(buffer, DIGIT_ONE, bufferDigits, 6);
    refreshLCD(DIGIT_ONE, 6);
}

// Show values as a combined function
/**
 * Show the value on the display for radio using an integer.
 * This function will also set the dot for radio.
 * @param value The value to display
 */
void KAV_A3XX_RAD_TCAS_LCD::showRadio(uint32_t value)
{
    char bufferDigits[12] = {0};
    if (value == 0)
        snprintf(bufferDigits, 12, "000.000");
    else
        dtostrf((float)value/1000, 6, 3, bufferDigits);
    showValue(bufferDigits);
}

/**
 * Show the value on the display for TCAS using an integer.
 * This will also clear all the dots.
 * @param value The value to display
 */
void KAV_A3XX_RAD_TCAS_LCD::showTcas(uint16_t value)
{
    char bufferDigits[12] = {0};
    if (value > 9999)
        value = 9999;
    snprintf(bufferDigits, 12, " %04d ", value);
    showValue(bufferDigits);
}

/**
 * Show the aircraft 'test' pattern on the display.
 * @param enabled Whether to show the test pattern (true) or hide it (false)
 */
void KAV_A3XX_RAD_TCAS_LCD::showTest(bool enabled)
{
    char bufferDigits[12] = {0};
    if (enabled)
        snprintf(bufferDigits, 12, "8.8.8.8.8.8");
    else
        snprintf(bufferDigits, 12, "      ");
    showValue(bufferDigits);
}

void KAV_A3XX_RAD_TCAS_LCD::showValue(char* value) {
    getDigitPattern(buffer, DIGIT_ONE, value, 6, (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5));
    refreshLCD(DIGIT_ONE, 6);
}


/**
 * Handle MobiFlight Commands
 * This function shouldn't be called be a user, it should only be called by the
 * custom device function. This is where data from MobiFlight enters the
 * library and is handled to be displayed on the LCD.
 * @param cmd The command from MobiFlight
 * @see handleMobiFlightRaw
 */
void KAV_A3XX_RAD_TCAS_LCD::set(int16_t messageID, char *setPoint)
{
    int32_t data = strtoul(setPoint, NULL, 10);
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
        setRadioDot((uint16_t)data);
    else if (messageID == 1)
        setAllDots((uint16_t)data);
    else if (messageID == 2)
        showRadio((uint32_t)data);
    else if (messageID == 3)
        showTcas((uint16_t)data);
    else if (messageID == 4)
        showValue(setPoint);
}
