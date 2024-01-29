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
void KAV_A3XX_EFIS_LCD::clearLCD()
{
    for (uint8_t i = 0; i < ht_efis.MAX_ADDR; i++)
        ht_efis.write(i, 0);
    memset(buffer, 0, BUFFER_SIZE_MAX);
}

// QFE, QNH and Dot Functions
void KAV_A3XX_EFIS_LCD::setQFE(bool enabled)
{
    SET_BUFF_BIT(DIGIT_THREE, 4, enabled);
    refreshLCD(DIGIT_THREE);
}

void KAV_A3XX_EFIS_LCD::setQNH(bool enabled)
{
    SET_BUFF_BIT(DIGIT_FOUR, 4, enabled);
    refreshLCD(DIGIT_FOUR);
}

void KAV_A3XX_EFIS_LCD::setDot(bool enabled)
{
    SET_BUFF_BIT(DIGIT_TWO, 4, enabled);
    refreshLCD(DIGIT_TWO);
}

void KAV_A3XX_EFIS_LCD::showStd(uint16_t state)
{
    if (state == 1) {
        displayDigit(DIGIT_ONE, 5);
        displayDigit(DIGIT_TWO, 11);
        displayDigit(DIGIT_THREE, 12);
        displayDigit(DIGIT_FOUR, 13);
    } else {
        displayDigit(DIGIT_ONE, 13);
        displayDigit(DIGIT_TWO, 13);
        displayDigit(DIGIT_THREE, 13);
        displayDigit(DIGIT_FOUR, 13);
    }
    setDot(false);
    setQFE(false);
    setQNH(false);
}

// Show Values
void KAV_A3XX_EFIS_LCD::showQNHValue(uint16_t value)
{
    if (value > 9999) value = 9999;
    if (value < 2000) {
        // If value is less than 2000, then it's hPa, so no decimal point.
        setDot(false);
    } else {
        // If value is greater than 2000, then it's inHg, so decimal point.
        setDot(true);
    }

    displayDigit(DIGIT_FOUR, (value % 10));
    value = value / 10;
    displayDigit(DIGIT_THREE, (value % 10));
    value = value / 10;
    displayDigit(DIGIT_TWO, (value % 10));
    displayDigit(DIGIT_ONE, (value / 10));

    setQFE(false);
    setQNH(true);
}

void KAV_A3XX_EFIS_LCD::showQFEValue(uint16_t value)
{
    if (value > 9999) value = 9999;
    if (value < 2000) {
        // If value is less than 2000, then it's hPa, so no decimal point.
        setDot(false);
    } else {
        // If value is greater than 2000, then it's inHg, so decimal point.
        setDot(true);
    }

    displayDigit(DIGIT_FOUR, (value % 10));
    value = value / 10;
    displayDigit(DIGIT_THREE, (value % 10));
    value = value / 10;
    displayDigit(DIGIT_TWO, (value % 10));
    displayDigit(DIGIT_ONE, (value / 10));

    setQFE(true);
    setQNH(false);
}

void KAV_A3XX_EFIS_LCD::showQFE_QNHValue(char* value)
{
    displayString(DIGIT_ONE, value, 4);
}

// Global Functions
uint8_t digitPatternEFIS[14] = {
    0b11101011, // 0
    0b01100000, // 1
    0b11000111, // 2
    0b11100101, // 3
    0b01101100, // 4
    0b10101101, // 5 or S
    0b10101111, // 6
    0b11100000, // 7
    0b11101111, // 8
    0b11101101, // 9
    0b00000100, // -
    0b00001111, // t
    0b01100111, // d
    0b00000000, // blank
};

void KAV_A3XX_EFIS_LCD::displayDigit(uint8_t address, uint8_t digit)
{
    // This ensures that anything over 12 is turned to 'blank', and as it's unsigned, anything less than 0 will become 255, and therefore, 'blank'.
    if (digit > 13) digit = 13;

    buffer[address] = (buffer[address] & 16) | digitPatternEFIS[digit];

    refreshLCD(address);
}

void KAV_A3XX_EFIS_LCD::displayString(uint8_t address, char* digits, uint8_t maxDigits)
{
    uint8_t digitCount = 0;
    uint8_t charCount = 0;
    uint8_t dpDigitMask = (1<<1);       // decimal point only on Digit 1 (2nd Digit)

    do {
        // Clear decimal point if allowed
        if ((1 << digitCount) && dpDigitMask)
            buffer[address + digitCount] = 0x00;
        else
            buffer[address + digitCount] &= 0x10;

        // Write character to buffer
        buffer[address + digitCount] |= readCharFromFlash((uint8_t)digits[charCount]);
        // Check if next character is a decimal point
        if (digits[charCount + 1] == '.') {
            // set decimal point only if allowed
            if (digitCount == 1)
                buffer[address + digitCount] |= 0x10;
            // step to next character
            charCount++;
        }
        digitCount++;
        charCount++;
    } while (digits[charCount] != 0x00 && digitCount < maxDigits);

    // Refresh LCD area
    for (uint8_t i = 0; i < maxDigits; i++) {
        refreshLCD(address + i);
    }
}

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
        setQNH((bool)data);
    else if (messageID == 4)
        setQFE((bool)data);
    else if (messageID == 5)
        showQFE_QNHValue(setPoint);
}
