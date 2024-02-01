#include "KAV_A3XX_EFIS_LCD.h"
#include "KAV_ReadChar.h"

#define DIGIT_ONE   0
#define DIGIT_TWO   1
#define DIGIT_THREE 2
#define DIGIT_FOUR  3

#define SET_BUFF_BIT(addr, bit, enabled) buffer[addr] = (buffer[addr] & (~(1 << (bit)))) | (((enabled & 1)) << (bit))

void KAV_A3XX_EFIS_LCD::begin()
{
    ht_efis.begin();
    ht_efis.sendCommand(HT1621::RC256K);
    ht_efis.sendCommand(HT1621::BIAS_THIRD_4_COM);
    ht_efis.sendCommand(HT1621::SYS_EN);
    ht_efis.sendCommand(HT1621::LCD_ON);
    // This clears the LCD
    for (uint8_t i = 0; i < ht_efis.MAX_ADDR; i++)
        ht_efis.write(i, 0);

    // Initialises the buffer to all 0's.
    memset(buffer, 0, BUFFER_SIZE_MAX);
}

void KAV_A3XX_EFIS_LCD::attach(byte CS, byte CLK, byte DATA)
{
    _CS          = CS;
    _CLK         = CLK;
    _DATA        = DATA;
    _initialised = true;
    begin();
}
void KAV_A3XX_EFIS_LCD::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

void KAV_A3XX_EFIS_LCD::refreshLCD(uint8_t address)
{
    ht_efis.write(address * 2, buffer[address], 8);
}

void KAV_A3XX_EFIS_LCD::refreshLCD(uint8_t address, uint8_t digits)
{
    for (uint8_t i = 0; i < digits; i++) {
        refreshLCD(address + i);
    }
}

void KAV_A3XX_EFIS_LCD::clearLCD()
{
    for (uint8_t i = 0; i < ht_efis.MAX_ADDR; i++)
        ht_efis.write(i, 0);
    memset(buffer, 0, BUFFER_SIZE_MAX);
}

// QFE, QNH and Dot Functions
void KAV_A3XX_EFIS_LCD::setQFElabel(bool enabled)
{
    SET_BUFF_BIT(DIGIT_THREE, 4, enabled);
    refreshLCD(DIGIT_THREE);
}

void KAV_A3XX_EFIS_LCD::setQNHlabel(bool enabled)
{
    SET_BUFF_BIT(DIGIT_FOUR, 4, enabled);
    refreshLCD(DIGIT_FOUR);
}

void KAV_A3XX_EFIS_LCD::setDot(bool enabled)
{
    SET_BUFF_BIT(DIGIT_TWO, 4, enabled);
    refreshLCD(DIGIT_TWO);
}


// Show Values

void KAV_A3XX_EFIS_LCD::showStd(uint16_t state)
{
    if (state == 1) {
        getDigitPattern(buffer, DIGIT_ONE, (char*)"Std ", 4);
    } else {
        getDigitPattern(buffer, DIGIT_ONE, (char*)"    ", 4);
    }
    refreshLCD(DIGIT_ONE, 4);
    setDot(false);
    setQFElabel(false);
    setQNHlabel(false);
}

// value as uint16_t
// value = 2992 for hPA
// value = 1023 for mBar
void KAV_A3XX_EFIS_LCD::showQNHValue(uint16_t value)
{
    showQFEQNHValue(value);
    setQFElabel(false);
    setQNHlabel(true);
}

// value as float
// value = 29.92 for hPA
// value = 1023 for mBar
void KAV_A3XX_EFIS_LCD::showQNHValue(float value)
{
    char bufferDigits[10] = {0};

    if (value > 9999) value = 9999;

    sprintf(bufferDigits, "%05.2f", value);
    showQFEQNHValue(bufferDigits);

    setQFElabel(false);
    setQNHlabel(true);
}

// value as string
// value = 29.92 for hPA
// value = 1023 for mBar
void KAV_A3XX_EFIS_LCD::showQNHValue(char* value)
{
    getDigitPattern(buffer, DIGIT_ONE, value, 4, (1<<1));
    refreshLCD(DIGIT_ONE, 4);

    setQFElabel(false);
    setQNHlabel(true);
}

// value as uint16_t
// value = 2992 for hPA
// value = 1023 for mBar
void KAV_A3XX_EFIS_LCD::showQFEValue(uint16_t value)
{
    showQFEQNHValue(value);
    setQFElabel(true);
    setQNHlabel(false);
}

// value as float
// value = 29.92 for hPA
// value = 1023 for mBar
void KAV_A3XX_EFIS_LCD::showQFEValue(float value)
{
    char bufferDigits[10] = {0};

    if (value > 9999) value = 9999;

    sprintf(bufferDigits, "%05.2f", value);
    showQFEQNHValue(bufferDigits);

    setQFElabel(true);
    setQNHlabel(false);
}

// value as string
// value = 29.92 for hPA
// value = 1023 for mBar
void KAV_A3XX_EFIS_LCD::showQFEValue(char* value)
{
    getDigitPattern(buffer, DIGIT_ONE, value, 4, (1<<1));
    refreshLCD(DIGIT_ONE, 4);

    setQFElabel(true);
    setQNHlabel(false);
}

// value as string
// value = 29.92 for hPA
// value = 1023 for mBar
// no labels get set
void KAV_A3XX_EFIS_LCD::showQFEQNHValue(char* value)
{
    getDigitPattern(buffer, DIGIT_ONE, value, 4, (1<<1));
    refreshLCD(DIGIT_ONE, 4);
}

// value as uint16_t
// value = 29.92 for hPA
// value = 1023 for mBar
// no labels get set
void KAV_A3XX_EFIS_LCD::showQFEQNHValue(uint16_t value)
{
    char bufferDigits[10] = {0};

    if (value > 9999) value = 9999;
    if (value > 2000) {
        sprintf(bufferDigits, "%05.2f", (float)value/100 );
    } else {
        sprintf(bufferDigits, "%04d", value);
    }
    showQFEQNHValue(bufferDigits);
}

// Global Functions

void KAV_A3XX_EFIS_LCD::set(int16_t messageID, char *setPoint)
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
        showQNHValue((uint16_t)data);
    else if (messageID == 1)
        showQFEValue((uint16_t)data);
    else if (messageID == 2)
        showStd((uint16_t)data);
    else if (messageID == 3)
        setQNHlabel((bool)data);
    else if (messageID == 4)
        setQFElabel((bool)data);
    else if (messageID == 5)
        showQFEQNHValue(setPoint);
}
