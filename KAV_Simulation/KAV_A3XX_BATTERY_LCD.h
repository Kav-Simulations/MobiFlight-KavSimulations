/**
 * Kav A3XX BATTERY LCD
 * Written by: James Kavanagh & Keith Greenwood
 * This library has been written to drive the 'Kav Simulations' EFIS LCD Display.
 */

#pragma once

#define BUFFER_SIZE_MAX 16

#include "Arduino.h"
#include "HT1621.h"

class KAV_A3XX_BATTERY_LCD {
private:
  // Fields
  HT1621 ht_battery;
  uint8_t buffer[BUFFER_SIZE_MAX];
  bool _initialised;
  byte _CS;
  byte _CLK;
  byte _DATA;
  char _lastBattValue[10] = {};

  // Methods
  void refreshLCD(uint8_t address);
  void refreshLCD(uint8_t address, uint8_t digits);
  
public:
  // Constructor
  // 'CLK' is sometimes referred to as 'RW'
  KAV_A3XX_BATTERY_LCD(uint8_t CS, uint8_t CLK, uint8_t DATA) : ht_battery(CS, CLK, DATA) { };

  void begin();
  void clearLCD();
  void clearDigit(uint8_t address);
  void attach(byte CS, byte CLK, byte DATA);
  void detach();
  void set(int16_t messageID, char *setPoint);
  void setPowerSave(bool enabled);

  // Set 'V' function
  void setVoltSymbol(bool enabled);
  // Set Dot Function
  void setDot(bool enabled);
  // Set value function
  void setValue(uint16_t value);
  void setValue(float value);

  // Show BATTERY Value function
  void showBattValue(uint16_t value);
  void showBattValue(float value);
  void showBattValue(char* value);

  // Annunciator Test Function
  void setAnnunciatorTest(bool enabled);
};
