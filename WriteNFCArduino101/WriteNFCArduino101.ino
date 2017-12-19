/****************************************************************************/
/*
 * WriteNFCArduino101
 * Created by Manuel Montenegro, December 19, 2017.
 * Developed by Manuel Montenegro for Final Year Project. 
 * 
 *  This sketch attempts to write blocks on Mifare Classic 1k NFC card.
 *  
 *  Serial port works at 115200 baudrate because it's necessary print out the
 *  data and read from the Mifare card at the same time.
 *  
*/
/****************************************************************************/


#include <Adafruit_PN532_mod.h> // TODO: change this library for self-made library

// In Adafruit PN532 Shield, IRQ pin is attached to digital pin 2
#define PN532_IRQ   2
#define PN532_RESET 3 // Not connected by default on the NFC Shield

// Each authentication key type has a ID number.
#define keyAType  0   //TODO: this should be in library
#define keyBType  1

// Authentication keys used for Mifare Classic 1k card
uint8_t keyA[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t keyB[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// This object is used to manage PN532 module
Adafruit_PN532 nfc (PN532_IRQ, PN532_RESET);



void setup() {
  
  // Setting serial port baudrate
  Serial.begin (115200);

  // Wait until Serial port with PC is stablished (only necessary with Leonardo)
  while (!Serial);
  
  // I2C initialization & reset the PN532 module
  nfc.begin();

  // Check if PN532 is correctly connected to Arduino pins
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.println("Didn't find PN532 board");
    while (1); // halt
  }

  // Configure PN532 module to read RFID tags
  nfc.SAMConfig();  // Configure the Secure Access Module

}



void loop() {
  uint8_t success;                          // Flag to check if there was an error with PN532 and card communication
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint32_t blockNumber = 4;                 // block that is going to be written
  uint8_t block [] = { 0x4D, 0x61, 0x6E, 0x75, 0x65, 0x6C, 0x4D, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x65, 0x67, 0x72, 0x6F };

  Serial.println ("Place a Mifare Classic card on the reader and press any key...");
  // Wait for user input before proceeding
  while (!Serial.available());
  // Maybe more than a character was send...
  while (Serial.available()) {
    Serial.read();
    delay(1);
  }

  // Check if communication with the card is OK and retrieve card UID
  success = nfc.readPassiveTargetID( PN532_MIFARE_ISO14443A, uid, &uidLength );

  if (success) {

    // Mifare Classic Card has a 4 bytes UID
    if (uidLength != 4) {
      Serial.println("This doesn't seem to be a Mifare Classic Card!");
      return;
    }
    
    // Trying to authenticate the block memory
    success = nfc.mifareclassic_AuthenticateBlock( uid, uidLength, blockNumber, keyBType, keyB );

    if (!success) {
      
      Serial.println("Authentication failed");
      
    } else {

      // Write a single block in Mifare Classic 1k card
      nfc.mifareclassic_WriteDataBlock (blockNumber, block);
      
    }

  }

}
