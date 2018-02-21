#include <AT24CX.h>            // I2C EEPROM in RTC module management library

#define I2C_EEPROM_ADDR    0x57    // I2C Address of EEPROM integrated in RTC module

AT24CX i2cEeprom;



int address = 0;
byte value;

void setup() {

  i2cEeprom=AT24C32(I2C_EEPROM_ADDR);  // Inits I2C EEPROM in RTC module in I2C address
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

}

void loop() {

    // read a byte from the current address of the EEPROM
  value = i2cEeprom.read(address);

  Serial.print(address);
  Serial.print("\t");
  Serial.print(value, HEX);
  Serial.println();

  address = address + 1;
  if (address == 4096) {
    address = 0;
  }

  delay(5);

}
