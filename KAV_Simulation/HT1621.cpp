/**
 * \file HT1621.cpp
 * \brief Implementation of a class for dealing with the Holtek HT1621 chip.
 * \author Enrico Formenti
 * \copyright BSD license, check the License page on the blog for more information. All this text must be
 *  included in any redistribution.
 *  <br><br>
 *  See macduino.blogspot.com for more details.
 */

#include "HT1621.h"

void HT1621::begin()
{
    pinMode(_DATA_pin, OUTPUT);
    pinMode(_RW_pin, OUTPUT);
    pinMode(_CS_pin, OUTPUT);

    digitalWrite(_CS_pin, HIGH);
    digitalWrite(_RW_pin, HIGH);
    digitalWrite(_DATA_pin, HIGH);

#ifndef __HT1621_READ
    for (uint8_t i = 0; i < MAX_ADDR; i++)
        ram[i] = 0;
#endif
}

// OCIO !!!
// nell'esempio dopo ogni write viene dato un delay
// di 20 microsecondi...
void HT1621::writeBits(uint8_t data, uint8_t cnt)
{
    for (uint8_t i = 0; i < cnt; i++, data <<= 1) {
        // minimum OutputPulseWidth is 3.34 microseconds for writing data on 3V
        // data is read from the chip on the rising edge from _RW_pin
        digitalWrite(_RW_pin, LOW);
        digitalWrite(_DATA_pin, data & 0x80 ? HIGH : LOW);
        delayMicroseconds(4);
        digitalWrite(_RW_pin, HIGH);
        delayMicroseconds(4);
    }
}

void HT1621::writeBitsReverse(uint32_t data, uint8_t cnt)
{
    for (uint8_t i = 0; i < cnt; i++, data >>= 1) {
        digitalWrite(_RW_pin, LOW);
        digitalWrite(_DATA_pin, data & 1 ? HIGH : LOW);
        delayMicroseconds(4);
        digitalWrite(_RW_pin, HIGH);
        delayMicroseconds(4);
    }
}

#ifdef __HT1621_READ

uint8_t HT1621::readBits(uint8_t cnt)
{
    pinMode(_DATA_pin, INPUT);

    uint8_t data = 0;
    for (uint8_t i = 0; i < cnt; data <<= 1, i++) {
        // minimum InputPulseWidth is 6.67 microseconds for reading data on 3V
        // data is valid from the chip on the rising edge from _RW_pin
        digitalWrite(_RW_pin, LOW);
        delayMicroseconds(7);
        data |= (digitalRead(_DATA_pin) == HIGH);
        digitalWrite(_RW_pin, HIGH);
        delayMicroseconds(7);
    }

    pinMode(_DATA_pin, OUTPUT);

    return data;
}

#endif

void HT1621::sendCommand(uint8_t cmd, bool first, bool last)
{
    if (first) {
        TAKE_CS();
        writeBits(COMMAND_MODE, 4);
    }

    writeBits(cmd, 8);

    if (last)
        RELEASE_CS();
}

void HT1621::write(uint8_t address, uint32_t bits, uint8_t bit_cnt)
{
    TAKE_CS();

    writeBits(WRITE_MODE, 3);
    writeBits(address << 2, 6); // send only 6 bit, starting from more significant
    writeBitsReverse(bits, bit_cnt);
#ifndef __HT1621_READ
    // ram[address] = bits; // TODO
#endif

    RELEASE_CS();
}

void HT1621::writeArray(uint8_t address, uint8_t *array, uint8_t cnt)
{
    TAKE_CS();

    writeBits(WRITE_MODE, 3);
    writeBits(address << 2, 6);
    for (uint8_t i = 0; i < cnt; i++) {
        writeBitsReverse(array[i], 4);
#ifndef __HT1621_READ
        ram[i] = array[i];
#endif
    }

    RELEASE_CS();
}

#ifdef __HT1621_READ

uint8_t HT1621::read(uint8_t address)
{
    uint8_t data;

    TAKE_CS();

    writeBits(READ_MODE, 3);
    writeBits(address << 3, 6);
    data = readBits(8);

    RELEASE_CS();

    return data;
}

void HT1621::read(uint8_t address, uint8_t *data, uint8_t cnt)
{
    TAKE_CS();

    writeBits(READ_MODE, 3);
    writeBits(address << 3, 6);
    for (uint8_t i = 0; i < cnt; i++)
        data[i] = readBits(8);

    RELEASE_CS();
}

#else

uint8_t HT1621::read(uint8_t address)
{
    return ram[address];
}

void HT1621::read(uint8_t address, uint8_t *data, uint8_t cnt)
{
    for (uint8_t i = 0; i < cnt; i++)
        data[i] = ram[address + i];
}

#endif
