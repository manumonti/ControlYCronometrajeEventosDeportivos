#include <AT24CX.h>            // I2C EEPROM in RTC module management library

#define I2C_EEPROM_ADDR    0x57    // I2C Address of EEPROM integrated in RTC module

AT24CX i2cEeprom;

int address = 0;
byte value;

void setup() {
  i2cEeprom=AT24C32(I2C_EEPROM_ADDR);  // Inits I2C EEPROM in RTC module in I2C address
// initialize the LED pin as an output.
  pinMode(13, OUTPUT);
}

void loop() {

  for (int i = 0; i < 4095; i++) {
    i2cEeprom.write(i, 0);
  }

  // turn the LED on when we're done
  digitalWrite(13, HIGH);

}
