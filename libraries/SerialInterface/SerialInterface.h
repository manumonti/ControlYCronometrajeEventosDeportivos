/*********************************************************************************************/
/*
 * Serial interface Arduino library
 * Created by Manuel Montenegro, January 20, 2017.
 * Developed for Manuel Montenegro Final Year Project. 
 * 
 *  This library is used for establishing a communication with PC by serial port and allows
 *	users to interact with Arduino easily. Serial interface is an abstraction opened to be 
 *	implemented. This library is only a simple implementation.
 *
 *	The use of this library is compatible with any serial port monitor, like Serial Monitor of
 *	Arduino IDE, GoSerial (Mac) or Putty (Windows).
*/
/*********************************************************************************************/

#ifndef __SERIALINTERFACE_H__
#define __SERIALINTERFACE_H__

#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class SerialInterface {
public:
	SerialInterface ();
	uint8_t introMenu ();
	uint8_t setupMenu (uint8_t stationID);

private:

	
};

#endif