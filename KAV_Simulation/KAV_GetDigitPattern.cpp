#include <Arduino.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#define pgm_read_byte(addr) \
    (*(const unsigned char *)(addr)) // workaround for non-AVR
#endif


// bit/segment sequence: ABCpFGED
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---   .d
//      D
const static byte digitPatternLong[128] PROGMEM = {
  // 0           ,1         ,2          ,3          ,4          ,5          ,6          ,7
    0b11101011, 0b01100000, 0b11000111, 0b11100101, 0b01101100, 0b10101101, 0b10101111, 0b11100000,
    // 8         ,9         ,A          ,B          ,C          ,D          ,E          ,F
    0b11101111, 0b11101101, 0b11101110, 0b11101111, 0b10001011, 0b00000000, 0b11001101, 0b10001110,
    // <0x10>    ,<0x11>    ,<0x12>     ,<0x13>     ,<0x14>     ,<0x15>     ,<0x16>     ,<0x17>
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // <0x18>    ,<0x19>    ,<0x1A>     ,<0x1B>     ,<0x1C>     ,<0x1D>     ,<0x1E>     ,<0x1F>
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (space)   ,!         ,"          ,#          ,$          ,%          ,&          ,'
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (         ,)          ,*         ,+          ,,          ,-          ,.          ,/
    0b10011010, 0b11110000, 0b00000000, 0b00000000, 0b10000000, 0b00000100, 0b00010000, 0b00000000,
    // 0         ,1         ,2          ,3          ,4          ,5          ,6          ,7
    0b11101011, 0b01100000, 0b11000111, 0b11100101, 0b01101100, 0b10101101, 0b10101111, 0b11100000,
    // 8         ,9         ,:          ,;          ,<          ,=          ,>          ,?
    0b11101111, 0b11101101, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // @         ,A         ,0b         ,C          ,D          ,E          ,F          ,G
    0b00000000, 0b11101110, 0b11101111, 0b10001011, 0b01100111, 0b11001101, 0b10001110, 0b10101111,
    // H         ,I         ,J          ,K          ,L          ,M          ,N          ,O
    0b01101110, 0b01100000, 0b01100001, 0b00000000, 0b00001011, 0b00000000, 0b00100110, 0b11101011,
    // P         ,Q         ,R          ,S          ,T          ,U          ,V          ,W
    0b11001110, 0b11101100, 0b00000110, 0b10101101, 0b00001111, 0b01101011, 0b00000000, 0b00000000,
    // X         ,Y         ,Z          ,[          ,\          ,]          ,^          ,_
    0b00000000, 0b01101101, 0b11000111, 0b10001011, 0b00000000, 0b11100001, 0b00000000, 0b00000001,
    // `         ,a         ,0b         ,c          ,d          ,e          ,f          ,g
    0b00000000, 0b11100111, 0b00101111, 0b00000111, 0b01100111, 0b11001111, 0b10001110, 0b11101101,
    // h         ,i         ,j          ,k          ,l          ,m          ,n          ,o
    0b00101110, 0b01100000, 0b01100001, 0b00000000, 0b00001011, 0b00000000, 0b00100110, 0b00100111,
    // p         ,q         ,r          ,s          ,t          ,u          ,v          ,w
    0b11001110, 0b11101100, 0b00000110, 0b10101101, 0b00001111, 0b00100011, 0b00000000, 0b00000000,
    // x         ,y         ,z          ,{          ,|          ,}          ,~          ,(delete)
    0b00000000, 0b01101101, 0b11000111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000
};


uint8_t readPatternFromFlash(uint8_t value) {
    return pgm_read_byte_near(digitPatternLong + (value & 0x7F));
}

void getDigitPattern(uint8_t *buffer, uint8_t address, char* digits, uint8_t maxDigits, uint8_t dpDigitMask)
{
    uint8_t digitCount = 0;
    uint8_t charCount = 0;

    do {
        if ((1 << digitCount) & dpDigitMask)        // Clear decimal point if allowed
            buffer[address + digitCount] = 0x00;
        else                                        //  otherwise keep it, special character
            buffer[address + digitCount] &= 0x10;

        buffer[address + digitCount] |= readPatternFromFlash((uint8_t)digits[charCount]);

        if (digits[charCount + 1] == '.') {         // check for decimal point
            if ((1 << digitCount) & dpDigitMask)    // set decimal point only if allowed
                buffer[address + digitCount] |= 0x10;
            charCount++;
        }
        digitCount++;
        charCount++;
    } while (digits[charCount] != 0x00 && digitCount < maxDigits);
}
