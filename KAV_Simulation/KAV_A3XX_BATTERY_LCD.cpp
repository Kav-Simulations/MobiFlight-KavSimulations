#include "KAV_A3XX_BATTERY_LCD.h"
#include "KAV_GetDigitPattern.h"

#define DIGIT_ONE 0
#define DIGIT_TWO 1
#define DIGIT_THREE 2

// Helper macro to set a specific bit in the buffer
#define SET_BUFF_BIT(addr, bit, enabled) buffer[addr] = (buffer[addr] & (~(1 << (bit)))) | (((enabled & 1)) << (bit));

/**
 * Setup the LCD
 * This function is called when the deivce is initialised using the 'attach' function.
 * It sets up the LCD and clears it.
 */
void KAV_A3XX_BATTERY_LCD::begin()
{
    ht_battery.begin();
    ht_battery.sendCommand(HT1621::RC256K);
    ht_battery.sendCommand(HT1621::BIAS_THIRD_4_COM);
    ht_battery.sendCommand(HT1621::SYS_EN);
    ht_battery.sendCommand(HT1621::LCD_ON);
    // This clears the LCD
    for (uint8_t i = 0; i < ht_battery.MAX_ADDR; i++)
        ht_battery.write(i, 0);

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
void KAV_A3XX_BATTERY_LCD::attach(byte CS, byte CLK, byte DATA)
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
void KAV_A3XX_BATTERY_LCD::detach()
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
void KAV_A3XX_BATTERY_LCD::refreshLCD(uint8_t address)
{
    ht_battery.write(address * 2, buffer[address], 8);
}

/**
 * Refresh the LCD
 * After a change is made to a segment, the display must be refreshed so that
 * the change is visible.
 * @param address The address to refresh
 * @param digits The number of digits to refresh
 */
void KAV_A3XX_BATTERY_LCD::refreshLCD(uint8_t address, uint8_t digits)
{
    for (uint8_t i = 0; i < digits; i++) {
        refreshLCD(address + i);
    }
}

/**
 * Clear the LCD
 * This function clears the LCD and resets the buffer.
 */
void KAV_A3XX_BATTERY_LCD::clearLCD()
{
    for (uint8_t i = 0; i < ht_battery.MAX_ADDR; i++)
        ht_battery.write(i, 0);
    memset(buffer, 0, BUFFER_SIZE_MAX);
}

void KAV_A3XX_BATTERY_LCD::clearDigit(uint8_t address)
{
    ht_battery.write(address * 2, 0);
    buffer[address] = 0;
}

// Voltage symbol and Dot Functions
/**
 * Set the voltage symbol to show 'V' or blank.
 * @param enabled Whether to show 'V' (true) or blank (false)
 */
void KAV_A3XX_BATTERY_LCD::setVoltSymbol(bool enabled)
{
    // Display 'V' if enable, or clear if !enabled
    // The left part of the 'V' is the 4th bit of the DIGIT_ONE address.
    // The right part of the 'V' is the 4th bit of the DIGIT_THREE address.
    SET_BUFF_BIT(DIGIT_ONE, 4, enabled);
    SET_BUFF_BIT(DIGIT_THREE, 4, enabled);
    refreshLCD(DIGIT_ONE);
    refreshLCD(DIGIT_THREE);
}

/**
 * Set the dot to show or hide.
 * @param enabled Whether to show the dot (true) or hide it (false)
 */
void KAV_A3XX_BATTERY_LCD::setDot(bool enabled)
{
    // The dot is the 4th bit of the DIGIT_TWO address.
    SET_BUFF_BIT(DIGIT_TWO, 4, enabled);
    refreshLCD(DIGIT_TWO);
}

/**
 * Set the value of the LCD using an integer.
 * @param value The value to display, 273 will be displayed as 27.3
 */
void KAV_A3XX_BATTERY_LCD::setValue(uint16_t value)
{
    setValue((float)value/10);
}

/**
 * Set the value of the LCD using an integer.
 * @param value The value to display, 27.3 will be displayed as 27.3
 */
void KAV_A3XX_BATTERY_LCD::setValue(float value)
{
    char bufferDigits[10] = {0};
    if (value > 99.9)
        value = 99.9;
    dtostrf(value, 4, 1, bufferDigits);
    showBattValue(bufferDigits);
}

// Show values as a combined function
/**
 * Show the value on the display with the 'V' character enabled using an integer.
 * @param value The value to display, 273 will be displayed as 27.3
 */
void KAV_A3XX_BATTERY_LCD::showBattValue(uint16_t value)
{
    showBattValue((float)value/10);
}

/**
 * Show the value on the display with the 'V' character enabled using an integer.
 * @param value The value to display, 27.3 will be displayed as 27.3
 */
void KAV_A3XX_BATTERY_LCD::showBattValue(float value)
{
    char bufferDigits[10] = {0};
    if (value > 99.9)
        value = 99.9;
    bufferDigits[0] = 'V';
    dtostrf(value, 4, 1, &bufferDigits[1]);
    showBattValue(bufferDigits);
}

// value as string, format as required
// V label get set if first character is 'V'
void KAV_A3XX_BATTERY_LCD::showBattValue(char* value)
{
    if (value[0] == 'V') {
        setVoltSymbol(true);
        getDigitPattern(buffer, DIGIT_ONE, &value[1], 3, (1<<1));
    } else {
        setVoltSymbol(false);
        getDigitPattern(buffer, DIGIT_ONE, value, 3, (1<<1));
    }
    refreshLCD(DIGIT_ONE, 3);
}

// Global Functions

/**
 * Handle MobiFlight Commands
 * This function shouldn't be called be a user, it should only be called by the
 * custom device function. This is where data from MobiFlight enters the
 * library and is handled to be displayed on the LCD.
 * @param cmd The command from MobiFlight
 * @see handleMobiFlightRaw
 */
void KAV_A3XX_BATTERY_LCD::set(int16_t messageID, char *setPoint)
{
    int32_t data = atoi(setPoint);
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
        setVoltSymbol((bool)data);
    else if (messageID == 1)
        setDot((bool)data);
    else if (messageID == 2)
        setValue((uint16_t)data);
    else if (messageID == 3)
        showBattValue((uint16_t)data);
    else if (messageID == 4)
        showBattValue(setPoint);
}
