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


#include <SerialInterface.h>

SerialInterface::SerialInterface () {

	Serial.setTimeout(100);				// Max timeout that serial port waits for data

}


// Master device sends the ID of next station and receives what want to do the user
uint8_t SerialInterface::sendStationIdReceiveChoice (uint8_t staID) {

	uint8_t choice;

	Serial.println(staID);				// Sends the station ID to PC

	while (!Serial.available());		// Waits until serial data is detected
	delay(10);							// Waits serial buffer receives all data
	choice = Serial.read();				// Saves user choice
	while (Serial.available()) {		// Cleans the serial buffer (return carriage)
		Serial.read();
	}

	return choice;

}


// Master device receives what want to do the user
uint8_t SerialInterface::receiveChoice () {

	uint8_t choice;

	while (!Serial.available());		// Waits until serial data is detected
	delay(10);							// Waits serial buffer receives all data
	choice = Serial.read();				// Saves user choice
	while (Serial.available()) {		// Cleans the serial buffer (return carriage)
		Serial.read();
	}

	return choice;
}


uint8_t SerialInterface::receiveChoiceSendAck () {

	uint8_t choice;

	while (!Serial.available());		// Waits until serial data is detected
	delay(10);							// Waits serial buffer receives all data
	choice = Serial.read();				// Saves user choice
	while (Serial.available()) {		// Cleans the serial buffer (i.e. return carriage)
		Serial.read();
	}
 
	Serial.println('1');				// Send ACK

	return choice;
}


// Master device receives user name by USB serial port
void SerialInterface::receiveName ( uint8_t *name ) {

	uint8_t count;						// A simple counter

	while (!Serial.available());		// Waits until serial data is detected
	delay(10);							// Waits serial buffer receives all data
	count = Serial.readBytes(name, NAME_SIZE-1);	// Read user name and saves it
	name[count]='\0';					// Put null terminated
	while (Serial.available()) {		// Clean the serial buffer
		Serial.read();
	}

}


// Master device receives user name by USB serial port
void SerialInterface::receiveCategory ( uint8_t *category) {

	uint8_t count;						// A simple counter

	while (!Serial.available());		// Waits until serial data is detected
	delay(10);							// Waits serial buffer receives all data
	count = Serial.readBytes (category, CAT_SIZE-1);// Read user category and saves it
	category[count]='\0';				// Put null terminated
	while (Serial.available()) {		// Clean the serial buffer
		Serial.read();
	}

}


// Master device recives date by Serial in format "mmm dd yyyy"
void SerialInterface::receiveDate ( uint8_t *dateReceived ) {

	while (!Serial.available());		// Waits until serial data is detected
	delay(10);							// Waits serial buffer receives all data
	Serial.readBytes(dateReceived, STRING_DATE_SIZE);// Read time and saves it

}


// Master device recives time by Serial in format "hh:mm:ss"
void SerialInterface::receiveTime ( uint8_t *timeReceived ) {

	while (!Serial.available());		// Waits until serial data is detected
	delay(10);							// Waits serial buffer receives all data
	Serial.readBytes(timeReceived, STRING_TIME_SIZE);// Read time and saves it

}


// Master device send a byte with any value by USB
void SerialInterface::sendChar (uint8_t character) {
	Serial.println(character);
}


// Master device sends a String by USB serial
void SerialInterface::sendString (uint8_t *string) {
	Serial.println((char*)string);
}



// Master device sends a hexadecimal array in the correct way
void SerialInterface::sendHexString(uint8_t array[], uint8_t len) {
	char buffer [(len*2)+1];
	for (uint8_t i = 0; i < len; i++) {
		byte nib1 = (array[i] >> 4) & 0x0F;
		byte nib2 = (array[i] >> 0) & 0x0F;
		buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
		buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
	}
	buffer[len*2] = '\0';

	Serial.println(buffer);
}


// Master device sends a String with data about a punch
void SerialInterface::sendPunchData (uint8_t ids, uint8_t *punchTime, uint8_t validated) {
	Serial.println (ids);
	if (punchTime[0] < 10) {
		Serial.print("0");
	}
	Serial.print (punchTime[0]);
	Serial.print (":");
	if (punchTime[1] < 10) {
		Serial.print("0");
	}
	Serial.print (punchTime[1]);
	Serial.print (":");
	if (punchTime[2] < 10) {
		Serial.print("0");
	}
	Serial.print (punchTime[2]);
	Serial.println ();

	if (validated) {
		Serial.println ("Ok!");
	} else {
		Serial.println ("Error!");
	}
}

// Send if there are more blocks to show
void SerialInterface::sendContinue (uint8_t continueWithBlocks) {
	if (continueWithBlocks) {
		Serial.println('1');
	} else {
		Serial.println('0');
	}
}	