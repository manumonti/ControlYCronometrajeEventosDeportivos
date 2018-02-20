/*********************************************************************************************/
/*
 * Stations Set Up Arduino library
 * Created by Manuel Montenegro, January 28, 2017.
 * Developed for Manuel Montenegro Bachelor Thesis. 
 * 
 *  This library is used for setting up the stations of the platform before a sport event.
 *	Some of this methods are destinated to Master device and the other ones are destinated to
 * 	Stations devices.
 *
 *	Stations' setup is done by NFC P2P.
 *
 *	Compatible boards with this library: Arduino UNO & Arduino Leonardo.
*/
/*********************************************************************************************/


#ifndef __SETUPSTATIONS_H__
#define __SETUPSTATIONS_H__


#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

													
#include <EEPROM.h>						// Arduino EEPROM management library
#include <RNG.h>						// Random Number Generator library
#include <SHA256.h>						// HMAC SHA256 library
#include <Curve25519.h>					// Diffie-Hellman library
#include <RTClib.h>						// Real Time Clock library
#include <AT24CX.h>						// I2C EEPROM in RTC module management library
#include <P2P-PN532.h>					// NFC P2P library
#include <SerialInterface.h>			// Serial communication with PC library


#define SETUP_TIMEOUT		5			// Max. time in seconds for start station setup
#define STRING_TIME_SIZE	8			// Size in bytes of time in format hh:mm:ss
#define STRING_DATE_SIZE	11			// Size in bytes of date in format mmm dd yyyy
#define TIME_SIZE			4			// Size in bytes of clock time
#define CHALLENGE_SIZE		16			// Size in bytes of generated challenge
#define KEY_SIZE			32			// Size in bytes of keys used
#define STATION_REC_SIZE	32			// Size in bytes of each station record
#define MASTER_RX_BUF_SIZE	32			// Max bytes of message that MASTER can receive
#define MASTER_TX_BUF_SIZE	49			// Max bytes of message that MASTER can send
#define NUM_STATIONS_ADDR	0			// EEPROM address where master saves the # of stations
#define RNG_SEED_ADDR		1			// EEPROM address where master & station saves RNGseed
#define SK_ADDR				50			// EEPROM address where master saves secret key
#define STATION_ID_ADDR		0			// EEPROM address where station saves its identifier
#define SHARED_KEY_ADDR		2			// EEPROM address where station saves its shared key
#define I2C_EEPROM_ADDR		0x57		// I2C Address of EEPROM integrated in RTC module
#define RNG_APP_TAG_MASTER	"master"	// Name unique of master for taking RNG seed
#define RNG_APP_TAG_STATI	"stati"		// Name unique of station for taking RNG seed


// Class for Master devices
class MasterSetUpStations {
public:
	MasterSetUpStations ();
	void startNewEvent ();				// Erases previous data of EEPROM & generates new keys
	void continuePreviousEvent();		// Loads data of previous event from EEPROM

private:
	P2PPN532 p2p;						// Manages NFC P2P connection
	SHA256 sha256;						// Manages HMAC & SHA256 functionalities
	RTC_DS3231 rtc;						// Manages Real Time Clock
	AT24CX i2cEeprom;					// Manages I2C EEPROM in RTC module
	SerialInterface usb;				// Serial Interface for communicating by USB port

	uint8_t stationID;					// ID of current station
	uint8_t challenge[CHALLENGE_SIZE];	// For stores the challenge
	uint8_t hmac [KEY_SIZE];			// HMAC received from station
	uint8_t masterPk [KEY_SIZE];		// Master Diffie-Hellman public key
	uint8_t masterSk [KEY_SIZE];		// Master Diffie-Hellman secret key
	uint8_t stationPk [KEY_SIZE];		// Station public key received

	void setUpProcess();				// Set up stations
	uint8_t sendP2P();					// Master sends to Station some data & a challenge 
	void receiveP2P();					// Master receives response from station by P2P
	void calculateSharedKey();			// Master calculates & stores the station key
	uint8_t checkHMAC ();				// Master checks received HMAC
};


// Class for Station devices
class StationNewSetUp {
public:
	StationNewSetUp();
	void startNewSetUp ();				// Erases previous data & start setup by NFC P2P


private:
	P2PPN532 p2p;						// Object that manages NFC P2P connection
	SHA256 sha256;						// Object that manages HMAC & SHA256 functionalities
	RTC_DS3231 rtc;						// Object that manages Real Time Clock

	uint8_t stationID;					// ID of current station
	uint8_t challenge[CHALLENGE_SIZE];	// Array for generating the challenge
	uint8_t stationPk [KEY_SIZE];		// Station Diffie-Hellman public key
	uint8_t stationSk_HMAC [KEY_SIZE];	// Station Diffie-Hellman secret key or HMAC
	uint8_t masterPk_Shared[KEY_SIZE];	// Can have two values: Master public key & shared key

	uint8_t receiveP2P();				// Waits until receivING challenge message or timeout
	void calculateHMAC();				// Calculates data received HMAC & station public key
	void sendP2P();						// Station sends to Master its public key & HMAC

};

#endif