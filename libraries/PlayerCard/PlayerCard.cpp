/*********************************************************************************************/
/*
 * Players' NFC card management Arduino library
 * Created by Manuel Montenegro, January 26, 2017.
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
PlayerCard::PlayerCard () : nfc( PN532_IRQ, PN532_RESET) {
	
	nfc.begin();   						// I2C initialization & resets PN532 module
  	nfc.SAMConfig();  					// Configures the Secure Access Module of PN532

}


// Erases previous information of card and set up it for a new event
void PlayerCard::format () {

	Serial.setTimeout(100);				// Max timeout that serial port waits for data

	uint8_t userChoice;					// Choose of user in Serial interface
	uint8_t uid[7];						// For storing card UID
	uint8_t nextBlock;					// Next block to be written in card
	uint8_t category [CAT_SIZE];		// Char array with player category
	uint8_t name [NAME_SIZE];			// Char array with player name
	uint8_t count;						// A simple counter

	readCardHeader(uid, &nextBlock, category, name);	// Reads info from card header

	// Sends card data by serial port
	printHexString(uid, 4);				// Sends uid
	Serial.println((char*)name);		// Sends user name
	Serial.println((char*)category);	// Sends user category

	while (!Serial.available());		// Waits until serial data is detected
	delay(10);							// Waits serial buffer receives all data
	userChoice = Serial.read();			// Saves user choice
	while (Serial.available()) {		// Clean the serial buffer
		Serial.read();
	}

	// If user wants to change name and category of card...
	if (userChoice == '1') {		
		// Reads new user name
		while (!Serial.available());		// Waits until serial data is detected
		delay(10);							// Waits serial buffer receives all data
		count = Serial.readBytes(name, NAME_SIZE-1);	// Read user name and saves it
		name[count]='\0';					// Put null terminated
		while (Serial.available()) {		// Clean the serial buffer
			Serial.read();
		}

		// Reads new user category
		while (!Serial.available());		// Waits until serial data is detected
		delay(10);							// Waits serial buffer receives all data
		count = Serial.readBytes (category, CAT_SIZE-1);// Read user category and saves it
		category[count]='\0';				// Put null terminated
		while (Serial.available()) {		// Clean the serial buffer
			Serial.read();
		}

		nextBlock = 4;						// In formatted card, first block is 4
		writeCardHeader (nextBlock, category, name);	// Writes the Card Header
	
	} else if (userChoice == '2') {
		
		nextBlock = 4;						// In formatted card, first block is 4
		writeCardHeader (nextBlock, category, name);	// Writes the Card Header
	
	}
}


// Reads data in first card's sector: ID, next memory block, category and player name
void PlayerCard::readCardHeader (uint8_t *uid, uint8_t *nb, uint8_t *cat, uint8_t *name ) {

	uint8_t uidLength;					// Length of the UID (depends on card type)
	uint8_t currentblock;				// Counter to keep track of actual block
	uint8_t success;					// Control flag
	uint8_t data[MIFARE_BLOCK_SIZE];	// For storing block data during reads

	currentblock = 1;					// First block in read is block #1

	// Waits until a valid card is placed on the reader and return readed UID
	success = nfc.readPassiveTargetID (PN532_MIFARE_ISO14443A, uid, &uidLength);

	// Checks if this is a Mifare Classic Card (UID length is 4)
	if (success && (uidLength == 4)) {

		// Authenticates the block's sector
    	if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, keyBType, keyb) ) {
			
			// Reads the data of the block and parse it
			if ( nfc.mifareclassic_ReadDataBlock(currentblock, data) ) {
				
				*nb = data[0];			// Saves next memory block
				for (uint8_t i = 0; i < CAT_SIZE; i++) {
					cat[i] = data[i+1];	// Saves category char array
				}
				currentblock ++;		// Updates to next block to read

			}
			if ( nfc.mifareclassic_ReadDataBlock(currentblock, data) ) {

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

	currentBlock = 1;					// First block in read is block #1

	// Waits until a valid card is placed on the reader and return readed UID
	success = nfc.readPassiveTargetID (PN532_MIFARE_ISO14443A, uid, &uidLength);

	// Checks if this is a Mifare Classic Card (UID length is 4)
	if (success && (uidLength == 4)) {

		// Authenticates the block's sector
    	if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentBlock, keyBType, keyb) ) {
			
			memcpy(data, &nb, sizeof(nb));	// Next free memory block
			memcpy(&data[1],cat,CAT_SIZE);	// User's category
			nfc.mifareclassic_WriteDataBlock (currentBlock, data); // Writes block in card

			currentBlock++;				// Take the next block

			memcpy(data, name, NAME_SIZE);	// User's name
			nfc.mifareclassic_WriteDataBlock (currentBlock, data);	//Writes next block

		}
	}

}


// Auxiliar function: Prints a hexadecimal array in the correct way
void PlayerCard::printHexString(uint8_t array[], unsigned int len) {
	char buffer [(len*2)+1];
    for (unsigned int i = 0; i < len; i++) {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';

    Serial.println(buffer);
}