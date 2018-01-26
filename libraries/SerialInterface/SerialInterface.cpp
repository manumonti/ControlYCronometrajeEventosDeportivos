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


#include <SerialInterface.h>

SerialInterface::SerialInterface () {

	Serial.setTimeout(100);				// Max timeout that serial port waits for data

}


