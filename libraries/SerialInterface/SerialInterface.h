/*********************************************************************************************/
/*
 * Serial interface Arduino library
 * Created by Manuel Montenegro, January 26, 2017.
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

class SerialInterface {
public:
	SerialInterface ();


private:


	
};

#endif