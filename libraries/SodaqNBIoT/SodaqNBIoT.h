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


#ifndef __SODAQNBIOT_H__
#define __SODAQNBIOT_H__


#include "Arduino.h"
#include "RTClib.h"


#define DEBUG 			Serial			// Serial port for DEBUG
#define UBLOX 			Serial3			// Serial port for comunnication with NB-IoT module
#define powerPin 		7				// Pin to turn on/off the NB-IoT module
#define networkOperator "21401"			// Vodafone network operator code


class SodaqNBIoT {
public:
	SodaqNBIoT ();
	bool begin ();						// Init serial port and turn module on
	String getIP();						// Return IP of ublox module
	String getIMEI();					// Return IMEI of card inserted in SODAQ module
	int openSocket (int port);			// Open an UDP socket in designated port
	bool sendData (String data, int sock, String ip, String port);	// Send data to ip and port
	bool sendPunch (uint8_t *data, uint8_t *idUser, int sock, String ip, String port);


private:
	String IP;							// IP that network give to ublox module
	String imei;						// IMEI of SIM card inserted in SODAQ module

	bool setUpUblox ( );				// Start sending AT commands for module configuration

	bool isAlive ();					// Check if ublox module responds to commands
	bool resetModule ();				// Reset ublox module and check OK in response
	bool confNbiotConnection ();		// Configure parameters for NB-IoT connection
	bool networkRegistration ();		// Select the operator and register SIM in network
	bool askForIp ();					// Take IP of ublox module from the network
	bool askForImei ();					// Take IMEI's SIM from ublox module

	bool checkRespForOk (int timeOut);	// Check if OK is in the response from ublox module 
	bool checkRespForReg (int timeOut);	// Check response looking for network registration
	String checkRespForIp(int timeOut);	// Check response looking for IP given by network
	int checkRespForSocket(int timeOut);// Check response looking for socket opened
	String checkRespForImei(int timeOut);// Check response looking for IMEI
	int checkRespForDataSended(int timeOut, int sock);	// Check # of bytes sent in response

	void sendIt (String atCommand);		// Send an AT command to ublox module
	String receiveIt ();				// Receive data sended by ublox module
	void printIt (String text);			// Print a string by debug serial port

	String stringToHexString(String str);// Conversion from string to hexadecimal string

	
};


#endif