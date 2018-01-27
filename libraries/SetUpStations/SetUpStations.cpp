/*********************************************************************************************/
/*
 * Stations Set Up Arduino library
 * Created by Manuel Montenegro, January 24, 2017.
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


#include <SetUpStations.h>


// MasterSetUpStations class methods ----------------------------------------------------------

// Class constructor
MasterSetUpStations::MasterSetUpStations () {
	RNG.begin (RNG_APP_TAG_MASTER, RNG_SEED_ADDR); // Saves new seed for generating random
	p2p.begin();						// Configures & resets PN532 module
	p2p.SAMConfiguration();				// Configures Secure Access Module of PN532 for P2P
	i2cEeprom=AT24C32(I2C_EEPROM_ADDR);	// Inits I2C EEPROM in RTC module in I2C address
	rtc.begin();						// Inits rtc object
}



// Deletes all previous information and invokes the setup process
void MasterSetUpStations::startNewEvent () {

	uint8_t receivedDate [STRING_DATE_SIZE];
	uint8_t receivedTime [STRING_TIME_SIZE];

	usb.receiveDate(receivedDate);		// Receives actual time from serial port
	usb.receiveTime(receivedTime);		// Receives actual time from serial port
	
	rtc.adjust(DateTime(receivedDate,receivedTime)); // Adjust time in RTC

	// Erases information of previous events
	EEPROM.update (NUM_STATIONS_ADDR, 0);	// Deletes the number of stations of previous event
	stationID = 0;						// Updates the variable of next station identifier

	// Generates a key pair for this event and saves Secret Key in EEPROM
	Curve25519::dh1 (masterPk, masterSk);	// Generates public and secret keys for this event
	EEPROM.put (SK_ADDR, masterSk);		// Saves master secret key in Arduino EEPROM

	setUpProcess ();					// Starts setting up new stations

}


// Loads previous information and invokes the setup process
void MasterSetUpStations::continuePreviousEvent () {
	
	EEPROM.get (NUM_STATIONS_ADDR, stationID);	// Take the next station ID for setup
	
	// Generates the master public key from EEPROM saved master secret key
	EEPROM.get (SK_ADDR, masterSk);		// Load from Arduino EEPROM master secret key
	Curve25519::eval (masterPk, masterSk, 0); // Generates master public key

	setUpProcess ();					// Starts setting up new stations

}


// Sets up each station one by one until user ends the process.
void MasterSetUpStations::setUpProcess () {


	uint8_t choice;						// User's choose
	uint8_t flag;						// Control flag

	choice = '1';						// Enters in the loop one time at least

	while ( choice == '1' ) {			// If user chooses set up a new station...

		choice = usb.sendStationIdReceiveChoice (stationID);

		if (choice == '1') {
			sendP2P ();					// Sends challenge to the station
			receiveP2P();				// Receives station public key and HMAC
			calculateSharedKey();		// Calculates keys of station & saves in I2C EEPROM
			flag = checkHMAC();			// Checks HMAC received

			if ( flag ) {
				// Ask for a new station or finish setup process 
				EEPROM [NUM_STATIONS_ADDR] += 1;	// Update the # of stations in Arduino EEPROM
				EEPROM.get (NUM_STATIONS_ADDR, stationID);	// Loads the next station number
				usb.sendChar('1');
			} else {
				usb.sendChar ('0');
			}

			
		} else {
			usb.sendChar (choice);
		}		
	}

}


/* Master device starts a communication with station by NFC P2P. Master sends the assigned 
  station ID, its public key and a challenge built with current time and random bytes.*/
uint8_t MasterSetUpStations::sendP2P () {

	uint8_t tx_buf [MASTER_TX_BUF_SIZE];// Buffer that will be sent
	uint32_t timeStamp;					// Buffer that will contain time stamp
	uint8_t randomNumber [CHALLENGE_SIZE - TIME_SIZE];	// Buffer for random generation
	uint8_t flag;						// Control flag

	EEPROM.get (NUM_STATIONS_ADDR, stationID);	// Take the next station ID for setup

	// Generates the challenge
	RNG.rand (randomNumber, sizeof(randomNumber));	// Random generation for challenge
	timeStamp = rtc.now().unixtime();	// Receives time from RTC the real time
	memcpy (challenge, &timeStamp, TIME_SIZE);	// Introduces time in challenge
	memcpy (&challenge[TIME_SIZE], randomNumber, sizeof(randomNumber));	//Introduces random

	// Makes the send buffer with all the information
	tx_buf[0] = stationID;				// Station identifier
	memcpy(&tx_buf[1], challenge, sizeof(challenge));	// Challenge
	memcpy(&tx_buf[17], masterPk, sizeof(masterPk));	// Master public key

	// Sends station ID, challenge and master public key by NFC P2P
	flag = false;
	while (!flag) {
		if (p2p.P2PInitiatorInit()) {	// Waits until the station is detected
			if (p2p.P2PInitiatorTxRx(tx_buf, sizeof(tx_buf), 0, 0)) {	// Sends data
				flag = true;
			}
		}
	}
	return 1;

}



/* Master receives station response by P2P NFC. This response should contain station public
key and a HMAC for validating the information sended and the station public key received*/
void MasterSetUpStations::receiveP2P() {
  
	uint8_t rx_buf [MASTER_RX_BUF_SIZE];// Buffer that will be received
	uint8_t rx_len;						// Size of data received
	uint8_t flag;						// Control flag

	// Receives station public key
	flag = false;
	while (!flag) {
		if (p2p.P2PTargetInit()) {		// Waits until the station is detected
			if (p2p.P2PTargetTxRx(0, 0, rx_buf, &rx_len)) {	// Waits data (public key)
				memcpy (stationPk, rx_buf, rx_len);	// Copies station public key in memory
				flag = true;      
			}
		}
	}

	// Receives Station HMAC
	flag = false;
	while (!flag) {
		if (p2p.P2PTargetInit()) {		// Waits until the station is detected
			if (p2p.P2PTargetTxRx(0, 0, rx_buf, &rx_len)) {	// Waits data (HMAC)
				memcpy (hmac, rx_buf, KEY_SIZE);	// Copies HMAC in memory
				flag = true;
			}
		}
	}  

}



// Master calculates the shared key with station public key received & saves it in I2C EEPROM
void MasterSetUpStations::calculateSharedKey() {

	uint8_t sharedKey [KEY_SIZE];		// Stores Diffie-Hellman shared key

	EEPROM.get (SK_ADDR, masterSk);		// Load from EEPROM master secret key
	memcpy (sharedKey, stationPk, sizeof(stationPk));	// Copies station public key.
	Curve25519::dh2 (sharedKey, masterSk);	// Generates Diffie-Hellman shared key

	// Saves the station key on I2C EEPROM
	i2cEeprom.write (stationID*STATION_REC_SIZE, sharedKey, STATION_REC_SIZE); 
  
}



/* Master calculates HMAC of stationID, challenge & station public key & checks it with 
receivedHMAC. Return true if calculated HMAC is equal to received or false if it isn't*/
uint8_t MasterSetUpStations::checkHMAC () {
  
	uint8_t calculatedHMAC [KEY_SIZE];	// Stores calculated HMAC for checking
	uint8_t sharedKey [KEY_SIZE];		// Key of the station
  
	// Saves the station key on I2C EEPROM
	i2cEeprom.read(stationID*STATION_REC_SIZE, sharedKey, STATION_REC_SIZE);
   
	// Calculating the HMAC
	sha256.resetHMAC(sharedKey, sizeof(sharedKey));	// Inits HMAC process
	sha256.update(&stationID, sizeof(stationID));	// Introduces station ID
	sha256.update(challenge, sizeof(challenge));	// Introduces challenge
	sha256.update(stationPk, sizeof(stationPk));	// Introduces station public key
	sha256.finalizeHMAC(sharedKey, sizeof(sharedKey), calculatedHMAC, sizeof(calculatedHMAC));

	// Check hmac calculated and received is the same
	if (  memcmp (hmac, calculatedHMAC, sizeof(hmac)) == 0  ) {
		return true;
	} else {
		return false;
	}  

}

// StationNewSetUp class methods --------------------------------------------------------------

// Class constructor
StationNewSetUp::StationNewSetUp () {

	rtc.begin();						// Inits rtc object
	p2p.begin();						// Configures & resets PN532 module
	p2p.SAMConfiguration();				// Configure the Secure Access Module of PN532 for P2P
	RNG.begin (RNG_APP_TAG_STATI, RNG_SEED_ADDR);	// Saves new seed for generating random
	Curve25519::dh1 (stationPk, stationSk_HMAC);	// Gen public-secret keys for this station

}


// Station erases previous data from EEPROM & start setup with data from Master by NFC P2P
void StationNewSetUp::startNewSetUp () {

	uint32_t realTime;					// For storing received real time

	receiveP2P ();						// Receives setup message and parse its data

	// Adjust RTC with the 4 firsts bytes from received challenge
	memcpy (&realTime, challenge, sizeof(realTime));// Challenge contains the real time
	rtc.adjust(realTime);				// Adjusts the RTC

	// Calculates the Diffie-Hellman shared key. This will be the station key
	Curve25519::dh2 (masterPk_Shared, stationSk_HMAC);	// Generates DH key & erases secret key
	EEPROM.put (STATION_ID_ADDR, stationID);			// Saves in EEPROM the station ID
	EEPROM.put (SHARED_KEY_ADDR, masterPk_Shared);		// Saves in EEPROM the shared key

	calculateHMAC ();					// Calculates HMAC & saves it in stationSk_HMAC
  
	sendP2P();							// Sends HMAC & Station public key to master
}


// Station waits until receives challenge message. Parse the data received in this message
void StationNewSetUp::receiveP2P () {
  
	uint8_t rx_buf [MASTER_TX_BUF_SIZE];// Buffer that will be received
	uint8_t rx_len;						// Size of data received
	uint8_t flag;						// Control flag

	// Receives challenge message. Doesn't send anything
	flag = false;
	while (!flag) {
		if(p2p.P2PTargetInit()){
			if(p2p.P2PTargetTxRx(0, 0, rx_buf, &rx_len)){  
				flag = true;
			}
		}
	}
  
	stationID = rx_buf[0];				// Station identifier
	memcpy (challenge, &rx_buf[1], sizeof(challenge));	// Challenge
	memcpy (masterPk_Shared, &rx_buf[17], sizeof(masterPk_Shared));	// Master public key

}


// Station calculates HMAC of stationID, challenge & station PK & stores it in stationSk_HMAC
void StationNewSetUp::calculateHMAC () {
  
	// Calculating the HMAC. Var stationSk_HMAC is reused because Its empty after DH2 function
	sha256.resetHMAC(masterPk_Shared, sizeof(masterPk_Shared));	// Inits HMAC process
	sha256.update(&stationID, sizeof(stationID));				// Introduces station ID
	sha256.update(challenge, sizeof(challenge));				// Introduces challenge
	sha256.update(stationPk, sizeof(stationPk));				// Introd. station public key
	sha256.finalizeHMAC(masterPk_Shared, sizeof(masterPk_Shared), stationSk_HMAC, sizeof(stationSk_HMAC));

}


/* Station sends to Master its public key & calculated HMAC of received data
for verificates public key*/
void StationNewSetUp::sendP2P () {
  
	uint8_t rx_buf [MASTER_TX_BUF_SIZE];// Buffer that will be received
	uint8_t rx_len;						// Size of data received
	uint8_t flag;						// Control flag
  
	// Sends station public key
	flag = false;
	while (!flag) {						// Retry the send if it fails
		if(p2p.P2PInitiatorInit()){
			if(p2p.P2PInitiatorTxRx(stationPk, sizeof(stationPk), rx_buf, &rx_len)){  
				flag = true;
			}
		}
	}

	// Sends calculated HMAC of station ID, challenge & station public key
	flag = false;
	while (!flag) {						// Retry the send if it fails
		if(p2p.P2PInitiatorInit()){
			if(p2p.P2PInitiatorTxRx(stationSk_HMAC, sizeof(stationSk_HMAC), rx_buf, &rx_len)){  
				flag = true;
			}
		}
	}

}