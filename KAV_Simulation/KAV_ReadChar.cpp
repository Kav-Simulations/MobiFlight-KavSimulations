#include <Arduino.h>
#include "KAV_ReadChar.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#define pgm_read_byte(addr) \
    (*(const unsigned char *)(addr)) // workaround for non-AVR
#endif


// bit/segment sequence: ???????p
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---   .d
//      D

const static byte digitPatternLongFCU[128] PROGMEM = {
    // 0         ,1         ,2          ,3          ,4          ,5          ,6          ,7
    0b11111010, 0b01100000, 0b10111100, 0b11110100, 0b01100110, 0b11010110, 0b11011110, 0b01110000,
    // 8         ,9         ,A          ,B          ,C          ,D          ,E          ,F
    0b11111110, 0b11110110, 0b01111110, 0b11111110, 0b10011010, 0b01111110, 0b10011110, 0b00011010,
    // <0x10>    ,<0x11>    ,<0x12>     ,<0x13>     ,<0x14>     ,<0x15>     ,<0x16>     ,<0x17>
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // <0x18>    ,<0x19>    ,<0x1A>     ,<0x1B>     ,<0x1C>     ,<0x1D>     ,<0x1E>     ,<0x1F>
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (space)   ,!         ,"          ,#          ,$          ,%          ,&          ,'
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (         ,)          ,*         ,+          ,,          ,-          ,.          ,/
    0b10011010, 0b11110000, 0b00000000, 0b00000000, 0b10000000, 0b00000100, 0b00000000, 0b00000000,
    // 0         ,1         ,2          ,3          ,4          ,5          ,6          ,7
    0b11111010, 0b01100000, 0b10111100, 0b11110100, 0b01100110, 0b11010110, 0b11011110, 0b01110000,
    // 8         ,9         ,:          ,;          ,<          ,=          ,>          ,?
    0b11111110, 0b11110110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // @         ,A         ,0b         ,C          ,D          ,E          ,F          ,G
    0b00000000, 0b01111110, 0b11111110, 0b10011010, 0b01111110, 0b10011110, 0b00011010, 0b11011110,
    // H         ,I         ,J          ,K          ,L          ,M          ,N          ,O
    0b01101110, 0b01100000, 0b11100000, 0b00000000, 0b10001010, 0b00000000, 0b00000000, 0b11111010,
    // P         ,Q         ,R          ,S          ,T          ,U          ,V          ,W
    0b00111110, 0b11110110, 0b00001100, 0b11010110, 0b10001110, 0b11101010, 0b00000000, 0b00000000,
    // X         ,Y         ,Z          ,[          ,\          ,]          ,^          ,_
    0b00000000, 0b11100110, 0b10111100, 0b10011010, 0b00000000, 0b11110000, 0b00000000, 0b10000000,
    // `         ,a         ,0b         ,c          ,d          ,e          ,f          ,g
    0b00000000, 0b11111100, 0b11001110, 0b10001100, 0b11101100, 0b10111110, 0b00011010, 0b11110110,
    // h         ,i         ,j          ,k          ,l          ,m          ,n          ,o
    0b01001110, 0b01100000, 0b11100000, 0b00000000, 0b00001010, 0b00000000, 0b01001100, 0b11001100,
    // p         ,q         ,r          ,s          ,t          ,u          ,v          ,w
    0b00111110, 0b11110110, 0b00001100, 0b11010110, 0b10001110, 0b11001000, 0b00000000, 0b00000000,
    // x         ,y         ,z          ,{          ,|          ,}          ,~          ,(delete)
    0b00000000, 0b11100110, 0b10111100, 0b00000000, 0b01100000, 0b00000000, 0b00000000, 0b00000000
};


uint8_t readFCUCharFromFlash(uint8_t value) {
    return pgm_read_byte_near(digitPatternLongFCU + (value & 0x7F));
}


void displayStringFCU(uint8_t *buffer, uint8_t address, char* digits, uint8_t maxDigits, uint8_t dpDigitMask)
{
    uint8_t digitCount = 0;
    uint8_t charCount = 0;
    uint8_t setDP = 0;

    // handle sign for VS/FPA is done in showVerticalFPAValue()
    do {
        buffer[address + digitCount] &= 0x01;
        buffer[address + digitCount] |= readFCUCharFromFlash((uint8_t)digits[charCount++]) | setDP;
        if (digits[charCount] == '.') {
            if ((1 << digitCount) & dpDigitMask) {
                setDP = 1;
        Serial.print("Digit: "); Serial.println(digitCount);
            } else {
                setDP = 0;
            }
            charCount++;
        }
        digitCount++;
    } while (digits[charCount] != 0x00 && digitCount < maxDigits);
}
