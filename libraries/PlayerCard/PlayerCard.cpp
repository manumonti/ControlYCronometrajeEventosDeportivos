/*********************************************************************************************/
/*
 * Players' NFC card management Arduino library
 * Created by Manuel Montenegro, January 28, 2017.
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
void PlayerCard::generateMac (uint8_t *mac, uint8_t uid, uint8_t ids, uint32_t time, uint8_t *lastBlockData ) {

	// Blake2s for authenticating the punch record
	blake.reset(stationKey, sizeof(stationKey), AUTH_IN_CARD_SIZE);
	blake.update(&uid, sizeof(uid));
	blake.update(&ids, sizeof(ids));
	blake.update(&time, sizeof(time));
	blake.update(lastBlockData, MIFARE_BLOCK_SIZE);
	blake.finalize(mac, AUTH_IN_CARD_SIZE);


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

		return FIRST_PUNCH_BLOCK;		// Return last card block

	} else if ( (cardBlock % 4) == 0 ){	// If last block number belong to sector trailer

		return cardBlock - 2;			// Avoiding sector trailer

	} else {

		return cardBlock - 1;			// Previous block is ok

	}

}




