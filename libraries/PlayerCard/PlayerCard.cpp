/*********************************************************************************************/
/*
 * Players' NFC card management Arduino library
 * Created by Manuel Montenegro, February 20, 2018.
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


#include <PlayerCard.h>


// Class constructor
PlayerCard::PlayerCard () : nfc( PN532_IRQ, PN532_RESET ) { }


// Inits the PN532 in Mifare card management mode
void PlayerCard::begin() {
	nfc.begin();						// I2C initialization & resets PN532 module
	nfc.SAMConfig();					// Configures the Secure Access Module of PN532
	i2cEeprom=AT24C32(I2C_EEPROM_ADDR);	// Inits I2C EEPROM in RTC module in I2C address
	rtc.begin();						// Inits Real Time Clock hardware

	EEPROM.get (ID_STATION_ADDR, idStation);	// Loads the assigned ID of this station
	EEPROM.get (KEY_EEPROM_ADDR, stationKey);	// Loads the assigned key of this station
}


// Master erases previous information of card and set up it for a new event
void PlayerCard::format () {

	uint8_t userChoice;					// Choose of user in Serial interface
	uint8_t uid[7];						// For storing card UID
	uint8_t nextBlock;
	uint8_t category [CAT_SIZE];		// Char array with player category
	uint8_t name [NAME_SIZE];			// Char array with player name

	readCardHeader(uid, &nextBlock, category, name);// Reads info from card header

	usb.sendHexString(uid,UID_LENGTH);	// Sends the UID of user's card
	usb.sendString (name);				// Sends user's name
	usb.sendString (category);			// Sends user's category

	userChoice = usb.receiveChoice();	// Saves user choice

	// If user wants to change name and category of card...
	if (userChoice == '1') {		
		
		usb.receiveName (name);			// Reads new user name
		usb.receiveCategory (category);	// Reads new user category

		writeCardHeader (FIRST_PUNCH_BLOCK, category, name);	// Writes the Card Header
	
	} else if (userChoice == '2') {

		writeCardHeader (FIRST_PUNCH_BLOCK, category, name);	// Writes the Card Header
	
	}
}





// Master reads all the punches in the card and checks its authentication code
void PlayerCard::readPunches () {

	uint8_t uid[7];						// UID of user's card
	uint8_t data[MIFARE_BLOCK_SIZE];	// For storing block data during reads
	uint8_t dataPrevBlock[MIFARE_BLOCK_SIZE];

	uint8_t lastBlock;					// Next free block in card & finish block
	uint8_t category [CAT_SIZE];		// Char array with player category
	uint8_t name [NAME_SIZE];			// Char array with player name
	uint8_t ids;						// Station identifier
	uint32_t punchTime;					// Time of current punch
	uint8_t punchTimeParsed [3];		// Time of current punch parsed
	uint8_t authCode[AUTH_IN_CARD_SIZE];// Authentication code
	uint8_t genMac[AUTH_IN_CARD_SIZE];	// Generated MAC for compare with auth code received

	uint8_t blockPointer;				// Pointer to the next card block to be readed
	uint8_t value;						// Auxiliar value for using memcpy function
	uint8_t punchAuthenticated;			// This will be true if the puch is authenticated



	readCardHeader(uid, &lastBlock, category, name);// Reads info from card header

	usb.sendHexString(uid,UID_LENGTH);	// Sends the UID of user's card
	usb.sendString (name);				// Sends user's name
	usb.sendString (category);			// Sends user's category


	blockPointer = FIRST_PUNCH_BLOCK;	// blockPointer starts pointing to first punch block


	while (blockPointer < lastBlock) {	// Reads each punch from card

		// The first punch is a special case, because the authentication is with block 1
		if (blockPointer == FIRST_PUNCH_BLOCK) {
			// BLOCK 1 was authenticated in readCardHeader method
			nfc.mifareclassic_ReadDataBlock(previousBlock(blockPointer), dataPrevBlock);
			// This is necessary because NB# change. For authentication must be 0
			dataPrevBlock[0] = 0;
			nfc.mifareclassic_AuthenticateBlock (uid, 4, blockPointer, keyBType, keyb);
			nfc.mifareclassic_ReadDataBlock(blockPointer, data);
		
		} else {						// If this isn't the first punch block

			// Check if this is a new block so that we can reauthenticate
			if (nfc.mifareclassic_IsFirstBlock(blockPointer)) {
				// Authenticates sector
				nfc.mifareclassic_AuthenticateBlock (uid, 4, blockPointer, keyBType, keyb);
			}

			// Read the current memory block
			nfc.mifareclassic_ReadDataBlock(blockPointer, data);

		}

		// Parse the data readed from the block
		ids = data [0];					
		memcpy(&punchTime, &data[1], sizeof(punchTime));
		memcpy(authCode, &data[5], AUTH_IN_CARD_SIZE);
		
		// Convert time in char array
		DateTime dateTime (punchTime);
		value = dateTime.hour();
		memcpy (&punchTimeParsed[0], &value, 1);
		value = dateTime.minute();
		memcpy (&punchTimeParsed[1], &value, 1);
		value = dateTime.second();
		memcpy (&punchTimeParsed[2], &value, 1);

		// Validates the punch
		// Reads the key of the stations that generated the punch
		loadStationKey (ids);
		generateMac (genMac, uid, ids, punchTime, dataPrevBlock);


		if (memcmp (genMac, authCode, sizeof(authCode)) == 0) {
			punchAuthenticated = true;
		} else {
			punchAuthenticated = false;
		}
		

		// Send the data by serial port
		usb.sendContinue (true);
		usb.sendPunchData (data[0], punchTimeParsed, punchAuthenticated);

		// Save the current block in previous block array
		memcpy (dataPrevBlock, data, sizeof(data));
		blockPointer = nextFreeBlock(blockPointer);	// Updates the value of blockPointer

	}

	usb.sendContinue (false);

  }




/* Station puts a punch record in user card with information about this control point.
The data saved in card is defined in documentation of this proyect*/
void PlayerCard::punch ( ) {

	uint8_t uid[7];						// UID of user's card
	uint8_t uidLength;					// Length of the UID (depends on card type)
	uint8_t cardBlock;					// For storing next card's block for writting
	uint8_t data[MIFARE_BLOCK_SIZE];	// For storing block data during reads
	uint8_t previousBlockData [MIFARE_BLOCK_SIZE];
	uint8_t success;					// Control flag

	// Waits until a valid card is placed on the reader and return readed UID
	success = nfc.readPassiveTargetID (PN532_MIFARE_ISO14443A, uid, &uidLength);
	// Checks if this is a Mifare Classic Card (UID length is 4)
	if (success && (uidLength == UID_LENGTH)) {

		// Reads what is the next free memory block
		// Authenticates the block's sector
		if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, NB_CAT_BLOCK, keyBType, keyb) ) {
			
			// Reads the data of the block and parse it
			if ( nfc.mifareclassic_ReadDataBlock(NB_CAT_BLOCK, data) ) {

				cardBlock = data[0];	// Saves next memory block number
				data[0] = nextFreeBlock (cardBlock); // Updates to next free block in card
				nfc.mifareclassic_WriteDataBlock (NB_CAT_BLOCK, data); // Saves changes in card

			}


			if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, previousBlock (cardBlock), keyBType, keyb) ) {
		
				// Reads the data of the block and parse it
				if ( nfc.mifareclassic_ReadDataBlock(previousBlock (cardBlock), previousBlockData) ) {

					/* This is necessary because NB# change along punches. So this must be
						constant for doing authentication */
					if (cardBlock == FIRST_PUNCH_BLOCK) {
						previousBlockData[0] = 0;
					}

					buildPunchRecord(cardBlock, previousBlockData, data, uid);	// Takes the punch record information

					// Writes punch in the next free memory block
					// Authenticates the block's sector
					if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, cardBlock, keyBType, keyb) ) {
						
						nfc.mifareclassic_WriteDataBlock (cardBlock, data);

					}
				}
			}
		}






	}


}


/* Station puts a punch record in user card with information about this control point.
The data saved in card is defined in documentation of this proyect*/
bool PlayerCard::punch (uint8_t *data, uint8_t *uid) {

	// uint8_t uid[7];						// UID of user's card
	uint8_t uidLength;					// Length of the UID (depends on card type)
	uint8_t cardBlock;					// For storing next card's block for writting
	// uint8_t data[MIFARE_BLOCK_SIZE];	// For storing block data during reads
	uint8_t previousBlockData [MIFARE_BLOCK_SIZE];
	uint8_t success;					// Control flag

	// Waits until a valid card is placed on the reader and return readed UID
	success = nfc.readPassiveTargetID (PN532_MIFARE_ISO14443A, uid, &uidLength);
	// Checks if this is a Mifare Classic Card (UID length is 4)
	if (success && (uidLength == UID_LENGTH)) {

		// Reads what is the next free memory block
		// Authenticates the block's sector
		if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, NB_CAT_BLOCK, keyBType, keyb) ) {
			
			// Reads the data of the block and parse it
			if ( nfc.mifareclassic_ReadDataBlock(NB_CAT_BLOCK, data) ) {

				cardBlock = data[0];	// Saves next memory block number
				data[0] = nextFreeBlock (cardBlock); // Updates to next free block in card
				nfc.mifareclassic_WriteDataBlock (NB_CAT_BLOCK, data); // Saves changes in card

			}


			if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, previousBlock (cardBlock), keyBType, keyb) ) {
		
				// Reads the data of the block and parse it
				if ( nfc.mifareclassic_ReadDataBlock(previousBlock (cardBlock), previousBlockData) ) {

					/* This is necessary because NB# change along punches. So this must be
						constant for doing authentication */
					if (cardBlock == FIRST_PUNCH_BLOCK) {
						previousBlockData[0] = 0;
					}

					buildPunchRecord(cardBlock, previousBlockData, data, uid);	// Takes the punch record information

					// Writes punch in the next free memory block
					// Authenticates the block's sector
					if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, cardBlock, keyBType, keyb) ) {
						
						nfc.mifareclassic_WriteDataBlock (cardBlock, data);

						return true;

					}
				}
			}
		}
	}

	return false;
}




/*Builds the punch record with necessary information: ID station, time stamp and HMAC
More information about punch block format can be found in documentation*/
void PlayerCard::buildPunchRecord ( uint8_t currentBlock, uint8_t *lastBlockData, uint8_t *block, uint8_t *uid ) {

	uint32_t timeStamp;					// Buffer that will contain time stamp
	uint8_t mac [AUTH_IN_CARD_SIZE];	// Result of poly authentication

	// ID of the station that is doing the punch record
	block[0] = idStation;

	// Time stamp of this punch
	timeStamp = rtc.now().unixtime();
	memcpy ( &block[1], &timeStamp, TIME_SIZE );	// Time stamp is put in punch

	generateMac (mac, uid, idStation, timeStamp, lastBlockData);

	memcpy ( &block[5], mac, AUTH_IN_CARD_SIZE );

}


/* Generates a Message Authentication code for a punch record
isFirstRecord must be 1 if is the first record or 0 if not*/
void PlayerCard::generateMac (uint8_t *mac, uint8_t *uid, uint8_t ids, uint32_t time, uint8_t *lastBlockData ) {

			// Serial.print("stationKey: ");
			// for (int i = 0; i < STATION_REC_SIZE; i++) {
			// 	Serial.print (stationKey[i],HEX);
			// 	Serial.print (" ");
			// }
			// Serial.println();
			// Serial.print("uid: ");
			// for (int i = 0; i < 4; i++) {
			// 	Serial.print (uid[i],HEX);
			// 	Serial.print (" ");
			// }
			// Serial.println();
			// Serial.println();
			// Serial.print("ids: ");
			// Serial.print(ids);
			// Serial.println();
			// Serial.print("time: ");
			// Serial.print(time);
			// Serial.println();
			// Serial.print("Last block data: ");
			// for (int i = 0; i < 16; i++) {
			// 	Serial.print (lastBlockData[i],HEX);
			// 	Serial.print (" ");
			// }
			// Serial.println();

	// Blake2s for authenticating the punch record
	blake.reset(stationKey, sizeof(stationKey), AUTH_IN_CARD_SIZE);
	blake.update(uid, 4);
	blake.update(&ids, sizeof(ids));
	blake.update(&time, sizeof(time));
	blake.update(lastBlockData, MIFARE_BLOCK_SIZE);
	blake.finalize(mac, AUTH_IN_CARD_SIZE);

			// Serial.print("MAC generated: ");
			// for (int i = 0; i < AUTH_IN_CARD_SIZE; i++) {
			// 	Serial.print (mac[i],HEX);
			// 	Serial.print (" ");
			// }
			// Serial.println();

}



// Return the next free block of user's card avoiding sector trailer's blocks
uint8_t PlayerCard::nextFreeBlock ( uint8_t cardBlock ) {

	if (cardBlock >= LAST_PUNCH_BLOCK){	// Memory full

		return LAST_PUNCH_BLOCK;// Return last card block

	} else if ( ((cardBlock+2) % 4) == 0 ){	// If next block number belong to sector trailer

		return cardBlock + 2;	// Avoiding sector trailer

	} else {

		return cardBlock + 1;	// Next block is ok

	}

}


// Return the last written block of user's card avoiding sector trailer's blocks
uint8_t PlayerCard::previousBlock ( uint8_t cardBlock ) {

	if (cardBlock <= FIRST_PUNCH_BLOCK){// This is the first block

		return NB_CAT_BLOCK;			// Return last card block

	} else if ( (cardBlock % 4) == 0 ){	// If last block number belong to sector trailer

		return cardBlock - 2;			// Avoiding sector trailer

	} else {

		return cardBlock - 1;			// Previous block is ok

	}

}


// Reads data in first card's sector: ID, next memory block, category and player name
void PlayerCard::readCardHeader ( uint8_t *uid, uint8_t *nb, uint8_t *cat, uint8_t *name ) {

	uint8_t uidLength;					// Length of the UID (depends on card type)
	uint8_t currentBlock;				// Counter to keep track of actual block
	uint8_t success;					// Control flag
	uint8_t data[MIFARE_BLOCK_SIZE];	// For storing block data during reads

	// Waits until a valid card is placed on the reader and return readed UID
	success = nfc.readPassiveTargetID (PN532_MIFARE_ISO14443A, uid, &uidLength);

	// Checks if this is a Mifare Classic Card (UID length is 4)
	if (success && (uidLength == UID_LENGTH)) {

		currentBlock = NB_CAT_BLOCK;	// First block in read

		// Authenticates the block's sector
		if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentBlock, keyBType, keyb) ) {
			
			// Reads the data of the block and parse it
			if ( nfc.mifareclassic_ReadDataBlock(currentBlock, data) ) {
				
				*nb = data[0];			// Saves next memory block
				for (uint8_t i = 0; i < CAT_SIZE; i++) {
					cat[i] = data[i+1];	// Saves category char array
				}

			}

			currentBlock = NAME_BLOCK;		// Updates to next block to read

			// Reads the data of the block and parse it
			if ( nfc.mifareclassic_ReadDataBlock(currentBlock, data) ) {

				for (uint8_t i = 0; i < NAME_SIZE; i++) {
					name[i] = data[i];	// Saves player name char array
				}

			}
		}
	}

}


// Writes data in first card's sector: next memory block, category and player name.
void PlayerCard::writeCardHeader (uint8_t nb, uint8_t *cat, uint8_t *name) {

	uint8_t uid [7];					// Returned UID of Mifare Card
	uint8_t uidLength;					// Length of the UID (depends on card type)
	uint8_t currentBlock;				// Counter to keep track of actual block
	uint8_t success;					// Control flag
	uint8_t data[MIFARE_BLOCK_SIZE];	// For storing block data during reads

	currentBlock = NB_CAT_BLOCK;		// First block in read is block #1

	// Waits until a valid card is placed on the reader and return readed UID
	success = nfc.readPassiveTargetID (PN532_MIFARE_ISO14443A, uid, &uidLength);

	// Checks if this is a Mifare Classic Card (UID length is 4)
	if (success && (uidLength == UID_LENGTH)) {

		// Authenticates the block's sector
    	if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentBlock, keyBType, keyb) ) {
			
			memcpy(data, &nb, sizeof(nb));	// Next free memory block
			memcpy(&data[1],cat,CAT_SIZE);	// User's category
			nfc.mifareclassic_WriteDataBlock (currentBlock, data); // Writes block in card

			currentBlock = NAME_BLOCK;	// Take the next block

			memcpy(data, name, NAME_SIZE);	// User's name
			nfc.mifareclassic_WriteDataBlock (currentBlock, data);	//Writes next block

		}
	}

}


void PlayerCard::loadStationKey (uint8_t ids) {
	uint8_t position = (ids*STATION_REC_SIZE);
	i2cEeprom.read(position, stationKey, STATION_REC_SIZE);

}



