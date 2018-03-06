/*********************************************************************************************/
/*
 * NB-IoT Arduino library
 * Created by Manuel Montenegro, March 01, 2018.
 * Developed for Manuel Montenegro Bachelor Thesis. 
 * 
 *  This library manages communication with SODAQ NB-IOT SHIELD.
 *
 *	Compatible boards with this library: Arduino MEGA.
*/
/*********************************************************************************************/


#include <SodaqNBIoT.h>

SodaqNBIoT::SodaqNBIoT () { 
	IP = "";
	imei = "";
}


// Start ublox n211 module
bool SodaqNBIoT::begin () {

	pinMode(powerPin, OUTPUT);
	digitalWrite(powerPin, HIGH);		// Turn the NB-IoT module on

	UBLOX.begin(9600);					// Start serial port with ublox
	DEBUG.begin(9600);					// Start serial port for debugging

	while(!UBLOX);						// Wait until ublox's serial connection is stablished

	return setUpUblox ( );				// Start the setting up of ublox module

}



// Return the IP that network assigns to the NB-IoT module
String SodaqNBIoT::getIP() {
	return IP;
}

// Return the IP that network assigns to the NB-IoT module
String SodaqNBIoT::getIMEI() {
	return imei;
}


// Open an UDP socket in port passed by parameter. Return the ID of socket
int SodaqNBIoT::openSocket (int port) {

	String atCommand = "AT+NSOCR=DGRAM,17,";
	atCommand += String (port);
	atCommand += ",1";

	sendIt(atCommand);

	return checkRespForSocket ( 5000 );

}


/* Send data to remote IP and Port. Data must be a String
	Return true if send data is correct or false if error */
bool SodaqNBIoT::sendData (String data, int sock, String ip, String port) {

	int dataSended;

	String atCommand ="AT+NSOST=";
	atCommand += sock;
	atCommand += ",";
	atCommand += ip;
	atCommand += ",";
	atCommand += port;
	atCommand += ",";
	atCommand += String(data.length());
	atCommand += ",";
	atCommand += stringToHexString(data);

	sendIt(atCommand);

	dataSended = checkRespForDataSended (500, sock);

	return (dataSended == ((int)data.length()));

}


/* Send punch to remote IP and Port. Data must be a String
	Return true if punch data is correct or false if error */
bool SodaqNBIoT::sendPunch (uint8_t *data, uint8_t *idUser, int sock, String ip, String port) {

	int dataSended;
	uint32_t punchTime;
	String atCommand = "";
	String message = "";

	memcpy (&punchTime, &data[1], sizeof(punchTime));

	DateTime dateTime (punchTime);

	message += "Player: ";
	for (int i = 0; i < 4; i++) {
		if ( idUser[i] < 0x10 ) {
			message += "0";
		}
		message += String (idUser[i],HEX);		
	}

	message += " | Station: ";
	message += String(data[0]);

	message += " | Time: ";
	if (dateTime.hour() < 10) {
		message += " ";
	}
	message += String(dateTime.hour());
	message += ":";
	if (dateTime.minute() < 10) {
		message += "0";
	}
	message += String(dateTime.minute());
	message += ":";
	if (dateTime.second() < 10) {
		message += "0";
	}
	message += String(dateTime.second());

	atCommand +="AT+NSOST=";
	atCommand += sock;
	atCommand += ",";
	atCommand += ip;
	atCommand += ",";
	atCommand += port;
	atCommand += ",";
	atCommand += String(message.length());
	atCommand += ",";
	atCommand += stringToHexString(message);

	sendIt(atCommand);

	dataSended = checkRespForDataSended (500, sock);

	return (dataSended == ((int)message.length()));

}








/* Set the NB-IoT module up sending AT commands. Register in the network and saves the IMEI 
	of the device.
	Return true if connection is successful or false otherwise*/
bool SodaqNBIoT::setUpUblox ( ) {

	bool flag;							// Flag for check the connection

	flag = isAlive();					// Check if there are connection with nbiot module

	while (!flag) {
		delay(500);
		flag = isAlive();
	}	

	if (flag) {
		flag = resetModule ();
	}
	if (flag) {
		flag = confNbiotConnection ();
	}
	if (flag) {
		flag = networkRegistration ();
	}
	if (flag) {
		flag = askForIp ();
	}
	if (flag) {
		flag = askForImei ();
	}

	return flag;

}



// Return true if ublox module responds to AT commands or false otherwise
bool SodaqNBIoT::isAlive() {

	sendIt ("AT");						// Send AT command

	return checkRespForOk ( 1000 );		// Check the response during time out

}


// Reset the module. Return true if successful
bool SodaqNBIoT::resetModule() {

	sendIt ("AT+NRB");					// Send AT command

	return checkRespForOk ( 10000 );	// Check the response during time out

}


// Configure parameters on the module for NB-IoT Connection
bool SodaqNBIoT::confNbiotConnection () {

	bool flag = true;					// Flag for checking correct working of AT commands

	if (flag) {
		sendIt("AT+CEREG=2");
		flag = checkRespForOk (500);
	}

	if (flag) {
		sendIt("AT+CSCON=0");
		flag = checkRespForOk (500);
	}
	if (flag) {
		sendIt("AT+CFUN=1");
		flag = checkRespForOk (6000);
	}
	if (flag) {
		sendIt("AT+CGDCONT=0,\"IP\",\"\"");
		flag = checkRespForOk (500);
	}

	return flag;

}


// Select the operator and register SIM in the network
bool SodaqNBIoT::networkRegistration () {

	String atCommand = "AT+COPS=1,2,\"";// Building AT Command
	atCommand += networkOperator;		// Network operator is defined in .h
	atCommand += "\"";
	sendIt (atCommand);					// Send AT command

	return checkRespForReg ( 180000 );	// Check the response during time out

}


// Saves the IP that NB-IoT network gives to ublox module
bool SodaqNBIoT::askForIp ( ) {

	String receivedIp;

	sendIt("AT+CGPADDR");				// Send AT command

	receivedIp = checkRespForIp (500);	// Receive and parse the IP

	if ( receivedIp.length() > 0 ) {
		IP = receivedIp;
		return true;
	} else {
		return false;
	}

}

// Saves the IMEI of SIM card
bool SodaqNBIoT::askForImei ( ) {

	String receivedImei;

	sendIt("AT+CGSN=1");				// Send AT command

	receivedImei = checkRespForImei (500);	// Receive and parse the IMEI

	if ( receivedImei.length() > 0 ) {
		imei = receivedImei;
		return true;
	} else {
		return false;
	}
}







/* Check for OK in the response from ublox module
	return true if OK is in the response or false if time out */
bool SodaqNBIoT::checkRespForOk ( int timeOut ) {

	String response;
	bool ok = false;
	unsigned long startTime = millis();	// Take time at start for time out

	// Check every char string received for "OK" response
	while ( !ok && ((millis()-startTime) <= (unsigned long) timeOut) ) {
		response = receiveIt();
		ok = (response.indexOf("\r\nOK\r\n") >= 0);	// Check if "OK" is in the response
	}

	return ok;
}


/* Check for network registration in the response from ublox module
	return true if registration is done or false if time out */
bool SodaqNBIoT::checkRespForReg ( int timeOut ) {

	String response;
	bool ok = false;
	unsigned long startTime = millis();	// Take time at start for time out

	// Check every char string received for "OK" response
	while ( !ok && ((millis()-startTime) <= (unsigned long) timeOut) ) {
		response = receiveIt();
		ok = (response.indexOf("\r\n+CEREG:5,") >= 0);	// Check if "OK" is in the response
	}

	return ok;
}



/* Check for IP given by network in the response from ublox module
	return the IP in string format */
String SodaqNBIoT::checkRespForIp ( int timeOut ) {

	String response;
	String receivedIp = "";
	bool ok = false;
	unsigned long startTime = millis();	// Take time at start for time out

	while ( !ok && ((millis()-startTime) <= (unsigned long) timeOut) ) {
		response += receiveIt();
		if (response.indexOf("\r\nOK\r\n") >= 0) {	// Check all the response is received
			if (response.indexOf("\r\n+CGPADDR:") >= 0) {
				receivedIp = response.substring((1+response.indexOf(",")));
				receivedIp = receivedIp.substring(0,receivedIp.indexOf("\r\n"));
			}
		}
	}

	return receivedIp;

}



/* Check for socket opened in the response from ublox module
	return the socket identifier or -1 if error*/
int SodaqNBIoT::checkRespForSocket ( int timeOut ) {

	String response;
	String receivedSocket;
	bool ok = false;
	unsigned long startTime = millis();	// Take time at start for time out

	while ( !ok && ((millis()-startTime) <= (unsigned long) timeOut) ) {
		response += receiveIt();

		if (response.indexOf("\r\nOK\r\n") >= 0) {	// Check all the response is received

			// Parse the response
			receivedSocket = response.substring(2+response.indexOf("\r\n"));
			receivedSocket = receivedSocket.substring(0, receivedSocket.indexOf("\r\n"));

			// Check if it's a valid value
			if (receivedSocket.toInt() >= 0 && receivedSocket.toInt() < 7) {
				ok = true;
			}

		}
	}

	if (ok) {
		return receivedSocket.toInt();
	} else {
		return -1;
	}

}


/* Check for IMEI's card in the response from ublox module
	Return the IMEI in string format */
String SodaqNBIoT::checkRespForImei ( int timeOut ) {

	String response;
	String receivedImei = "";
	bool ok = false;
	unsigned long startTime = millis();	// Take time at start for time out

	while ( !ok && ((millis()-startTime) <= (unsigned long) timeOut) ) {
		response += receiveIt();
		if (response.indexOf("\r\nOK\r\n") >= 0) {	// Check all the response is received
			if (response.indexOf("\r\n+CGSN:") >= 0) {
				receivedImei = response.substring((1+response.indexOf(":")));
				receivedImei = receivedImei.substring(0,receivedImei.indexOf("\r\n"));
			}
		}
	}

	return receivedImei;

}




/* Check for confirmation of correct sending in the response from ublox module.
	Return number of bytes ublox sent or -1 if error */
int SodaqNBIoT::checkRespForDataSended (int timeOut, int sock) {

	String response;
	String receivedSocket="";
	String receivedLength="";

	bool ok = false;
	unsigned long startTime = millis();	// Take time at start for time out

	while ( !ok && ((millis()-startTime) <= (unsigned long) timeOut) ) {
		response += receiveIt();
		if (response.indexOf("\r\nOK\r\n") >= 0) {	// Check all the response is received
			receivedSocket = response.substring((2+response.indexOf("\r\n")));
			receivedSocket = receivedSocket.substring(0,receivedSocket.indexOf(","));
			receivedLength = response.substring((1+response.indexOf(",")));
			receivedLength = receivedLength.substring(0,receivedLength.indexOf("\r\n"));
		}
	}

	if ((receivedSocket.toInt() == sock)) {
		return receivedLength.toInt();
	} else {
		return -1;
	}

}
	







// Send an AT command to ublox module
void SodaqNBIoT::sendIt ( String atCommand ) {

	printIt("-- "+atCommand);			// Print AT command sended
	UBLOX.print (atCommand+"\r");		// Send AT command to ublox module

}



// Receive the response from ublox module. Return it and print it by debug serial port
String SodaqNBIoT::receiveIt ( ) {

	String received;

	// Save data received from ublox module
	while (UBLOX.available()) {
		delay(10);						// Neccesary for don't split strings
		received += (char)UBLOX.read ();
	}

	printIt (received);					// DEBUG: Print response from ublox module

	return received;					// Return string received
  
}



// Print something by DEBUG serial port
void SodaqNBIoT::printIt ( String text ) {

	if (text.length() > 0) {			// If string is not empty...
		DEBUG.println(text);			// Send by debug serial port the string
	}
}



// Conversion from string to hexadecimal string
String SodaqNBIoT::stringToHexString (String str) {

	String hexString;

	char high_nibble;
	char low_nibble;

	for (int i = 0; i < str.length(); i++) {
		high_nibble = ((str[i] >> 4) & 0x0F);
		high_nibble = ((high_nibble <= 9) ? ('0' + high_nibble) : ('A' - 10 + high_nibble));
		low_nibble = (str[i] & 0x0F);
		low_nibble = ((low_nibble <= 9) ? ('0' + low_nibble) : ('A' - 10 + low_nibble));
		hexString += String(high_nibble);
		hexString += String(low_nibble);
	}

	return hexString;

}

