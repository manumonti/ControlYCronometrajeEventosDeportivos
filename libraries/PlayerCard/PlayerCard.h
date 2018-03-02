/*********************************************************************************************/
/*
 * Players' NFC card management Arduino library
 * Created by Manuel Montenegro, February 20, 2018.
 * Developed for Manuel Montenegro Bachelor Thesis. 
 * 
 *  This library is used for the management of players' cards. Allows operation like reading
 *	cards, writting in the next free memory block, etc.
 *	
 *	At the moment, this library only supports Mifare Classic 1k NFC Card.
 *
 *	Please, note that Arduino Leonardo uses digital pin 2 for I2C connection, so PN532 IRQ pin
 *  can cause a conflict and device won't work. Connect it to digital pin 10 in this case. 
 *	This is easy to do if you are using NFC Module by Elechouse. Otherwise, if you are using
 *	Adafruit PN532 RFID/NFC Shied, you can find out a well explained solution for this in:
 *	https://learn.adafruit.com/adafruit-pn532-rfid-nfc/shield-wiring
 *
 *	Compatible boards with this library: Arduino UNO & Arduino Leonardo.
*/
/*********************************************************************************************/


#ifndef __PLAYERCARD_H__
#define __PLAYERCARD_H__


#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif


#include <PN532.h>						// NFC Arduino library for manages Mifare cards
#include <BLAKE2s.h>					// Cryptographic Arduino Library for Blake2s
#include <RTClib.h>						// Real Time Clock library
#include <EEPROM.h>						// Arduino EEPROM management library
#include <SerialInterface.h>			// Serial communication with PC library
#include <AT24CX.h>						// I2C EEPROM in RTC module management library


#ifdef ARDUINO_AVR_LEONARDO
	#define PN532_IRQ   	10			// With Arduino Leonardo IRQ is tied to digital pin 10
#else
	#define PN532_IRQ   	2			// With others, IRQ is tied to digital pin 2
#endif

#define PN532_RESET     	12    		// Not connected by default
#define keyAType        	0			// Defines what mifare key type will be used in auth
#define keyBType        	1 			// Defines what mifare key type will be used in auth
#define POLY_NONCE_SIZE		16			// Size in bytes of Nonce in Poly1305
#define HMAC_KEY_SIZE		32			// Size in bytes of keys used for HMAC
#define TIME_SIZE			4			// Size in bytes of clock time
#define AUTH_IN_CARD_SIZE	11			// Size of HMAC in each punch record in user's card	
#define MIFARE_BLOCK_SIZE	16			// Size of each block on Mifare Classic 1k Card
#define STATION_REC_SIZE	32			// Size in bytes of each station record
#define UID_LENGTH			4			// Length of UID in Mifare Classic 1k cards
#define NB_CAT_BLOCK		1			// Block number of Next Block and Category in user card
#define NAME_BLOCK			2			// Block number of User Name in user card
#define FIRST_PUNCH_BLOCK	4			// Block number of first punch block in user card
#define LAST_PUNCH_BLOCK	62			// Block number of last block that can be written
#define ID_STATION_ADDR		0			// Arduino EEPROM address where is stored station ID
#define KEY_EEPROM_ADDR		50			// Arduino EEPROM address where is stored station Key
#define I2C_EEPROM_ADDR		0x57		// I2C Address of EEPROM integrated in RTC module


class PlayerCard {
public:
	PlayerCard ();
	void begin ();						// Inits the hardware
	void format ();						// Master formats player card erasing previous data	
	void readPunches ();				// Master reads & validates punches from card
	void punch ();						// Station puts information about this control point
	bool punch (uint8_t *data, uint8_t *uid);



private:
	PN532 nfc;							// Object that manages PN532 module
	BLAKE2s blake;						// Object that manages Blake2s crypto functionalities
	RTC_DS3231 rtc;						// Object that manages Real Time Clock
	SerialInterface usb;				// Serial Interface for communicating by USB port
	AT24CX i2cEeprom;					// Manages I2C EEPROM in RTC module

	// Key B for authenticates sectors of Mifare Classic Card
	const uint8_t keyb [6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	uint8_t idStation;					// ID of this station loaded from Arduino EEPROM
	uint8_t stationKey [HMAC_KEY_SIZE];	// Negociated key of this station

	void readCardHeader (uint8_t *uid, uint8_t *nb, uint8_t *cat, uint8_t *name );
	void writeCardHeader (uint8_t nb, uint8_t *cat, uint8_t *name);	// Writes card header info
	void buildPunchRecord ( uint8_t currentBlock, uint8_t *lastBlockData, uint8_t *block, uint8_t *uid );	// Builds the next record
	// Method that generates a message authentication code with blake2s
	void generateMac (uint8_t *mac, uint8_t *uid, uint8_t ids, uint32_t time, uint8_t *lastBlockData );
	uint8_t nextFreeBlock ( uint8_t cardBlock );// Return the following free block of card
	uint8_t previousBlock ( uint8_t cardBlock );// Return the last written block
	void loadStationKey (uint8_t ids);	// Master searchs in EEPROM the key for this IDS

};

#endif