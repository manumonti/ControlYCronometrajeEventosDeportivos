/*********************************************************************************************/
/*
 * Players' NFC card management Arduino library
 * Created by Manuel Montenegro, January 26, 2017.
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
#include <SerialInterface.h>			// Serial communication with PC library


#ifdef ARDUINO_AVR_UNO
	#define PN532_IRQ   	2			// With Arduino UNO IRQ is tied to digital pin 2
#elif ARDUINO_AVR_LEONARDO
	#define PN532_IRQ   	10			// With Arduino Leonardo IRQ is tied to digital pin 10
#endif

#define PN532_RESET     	3    		// Not connected by default
#define keyAType        	0			// Defines what mifare key type will be used in auth
#define keyBType        	1 			// Defines what mifare key type will be used in auth
#define MIFARE_BLOCK_SIZE	16			// Size of each block on Mifare Classic 1k Card
#define UID_LENGTH			4			// Length of UID in Mifare Classic 1k cards


class PlayerCard {
public:
	PlayerCard ();
	void format ();						// Formats player card erasing all previous data
	

private:
	PN532 nfc;							// Object that manages PN532 module
	SerialInterface usb;				// Serial Interface for communicating by USB port

	// Key B for authenticates sectors of Mifare Classic Card
	const uint8_t keyb [6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	// Reads data in first card's sector
	void readCardHeader (uint8_t *uid, uint8_t *nb, uint8_t *cat, uint8_t *name );
	void writeCardHeader (uint8_t nb, uint8_t *cat, uint8_t *name);	// Writes card header info

};

#endif