/*********************************************************************************************/
/*
 * Serial interface Arduino library
 * Created by Manuel Montenegro, January 26, 2018.
 * Developed for Manuel Montenegro Bachelor Thesis. 
 * 
 *  This library is used for establishing a communication with PC by serial port and allows
 *	users to interact with Arduino easily. Serial interface is an abstraction opened to be 
 *	implemented.
*/
/*********************************************************************************************/


#ifndef __SERIALINTERFACE_H__
#define __SERIALINTERFACE_H__

#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif


#define CAT_SIZE			15			// Size in bytes of category field in card
#define NAME_SIZE			16			// Size in bytes of player name field in card
#define STRING_TIME_SIZE	8			// Size in bytes of time in format hh:mm:ss
#define STRING_DATE_SIZE	11			// Size in bytes of date in format mmm dd yyyy


class SerialInterface {
public:
	SerialInterface ();					// Class constructor
	uint8_t sendStationIdReceiveChoice (uint8_t staID);	// Master sends to PC station ID
	uint8_t receiveChoice ();			// Master receives a byte with user's choice
	uint8_t receiveChoiceSendAck();		// Master receives user's choice and sends ACK
	void receiveName ( uint8_t *name );	// Master receives a user name for card by USB
	void receiveCategory ( uint8_t *category );	// Master receives a user name for card by USB
	void receiveTime ( uint8_t *timeReceived );	// Master receives time 
	void receiveDate ( uint8_t *dateReceived );	// Master receives date
	void sendChar (uint8_t character);	// Master sends a byte to PC
	void sendString (uint8_t *string);	// Master sends a string to PC
	void sendHexString(uint8_t array[], uint8_t len); // Master sends a hex string to PC
	void sendPunchData (uint8_t ids, uint8_t *punchTime, uint8_t validated); // Show punch
	void sendContinue (uint8_t continueWithBlocks);	// Send if there are more blocks to show

private:


};

#endif