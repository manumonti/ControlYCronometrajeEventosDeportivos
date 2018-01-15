/*********************************************************************************************/
/*
 * MASTER
 * Created by Manuel Montenegro, January 15, 2017.
 * Developed for Manuel Montenegro Final Year Project. 
 * 
 *  This sketch sets up station devices by NFC. It assignes an identifier (ID) and a shared 
 *  key using Diffie-Hellman algorithm. Persistent data is saved in EEPROM.
 * 
 *  This sketch must be uploaded in master device. Serial connection to PC and NFC shield is 
 *  required. Serial port baudrate: 115200
 *  
 *  Compatible boards with this sketch: Arduino UNO.
*/
/*********************************************************************************************/

#include <RNG.h>                    // Random Number Generator library
#include <Curve25519.h>             // Diffie-Hellman library
#include <P2P-PN532.h>              // NFC P2P library
#include <PN532.h>                  // NFC library
#include <EEPROM.h>                 // EEPROM management library
#include <SerialInterface.h>        // Interface with the user by serial port and PC library

#define KEY_SIZE            32      // Size in bytes of keys used
#define NUM_STATIONS_ADDR   0       // EEPROM address where is saved the number of stations
#define RNG_SEED_ADDR       1       // EEPROM address where is saved the seed for RNG
#define SK_ADDR             50      // EEPROM address where is saved secret key of the event
#define FIRST_REC_ADDR      82      // EEPROM address where begins stations records
#define STATION_REC_SIZE    33      // Size in bytes of each station record
#define RX_BUF_MAX_SIZE     32      // Max size in bytes of message that MASTER can receive
#define TX_BUF_MAX_SIZE     49      // Max size in bytes of message that MASTER can send
#define RNG_APP_TAG         "master"// Name unique of this app for taking RNG seed

P2PPN532 p2p;                       // Object that manages NFC P2P connection
SerialInterface serialInterface;    // Object that manages the user interface

uint8_t choose;                     // User choose from serial menu
uint8_t stationID;                  // ID of current station
uint8_t challenge [16];             // Array for generating the challenge

uint8_t masterPk [KEY_SIZE];        // Master Diffie-Hellman public key
uint8_t masterSk [KEY_SIZE];        // Master Diffie-Hellman secret key

uint8_t tx_buf [TX_BUF_MAX_SIZE];   // Buffer that will be sent
uint8_t rx_buf [RX_BUF_MAX_SIZE];   // Buffer that will be received
uint8_t rx_len;                     // Size of data received

uint8_t flag;                       // Control flag for multiples purpouses

// Sets up stations until it receives a finish command.
void setUpStations ( ) {

  p2p.begin();                      // Configures & resets PN532 module
  p2p.SAMConfiguration();           // Configure the Secure Access Module of PN532 for P2P
  
  choose = serialInterface.setupMenu(stationID);  // Starts setup interactive menu

  while ( choose == '1' ) {

    RNG.rand (challenge, sizeof(challenge));  // Generates the challenge for this station

    sendP2PChallenge ();      // Send challenge to the station

    receiveP2P();             // Receives response to challenge and station public key

    uint8_t EEPROMaddr = FIRST_REC_ADDR + (STATION_REC_SIZE * stationID); // Address of this station
    EEPROM.update (EEPROMaddr, stationID);  // Writes the station ID in EEPROM

    // Ask for a new station or finish setup process 
    EEPROM [NUM_STATIONS_ADDR] += 1;
    EEPROM.get (NUM_STATIONS_ADDR, stationID);
    choose = serialInterface.setupMenu (stationID);
    
  }

  choose = 0;                       // Put the flag in "invalid option"
  
}


// Send a challenge randomly generated
void sendP2PChallenge () {

  // Make the buffer with station information
  tx_buf[0] = stationID;            // Station identifier
  memcpy(&tx_buf[1], challenge, sizeof(challenge)); // Challenge
  memcpy(&tx_buf[17], masterPk, sizeof(masterPk));  // Master public key

  flag = false;
  while (!flag) {
    if (p2p.P2PInitiatorInit()) {         // Waits until the station is detected
      if (p2p.P2PInitiatorTxRx(tx_buf, sizeof(tx_buf), rx_buf, &rx_len)) { // Waits data
        flag = true;
      }
    }
  }
}


// Receives station response to P2P Challenge
void receiveP2P () {
  flag = false;
  while (!flag) {
    if (p2p.P2PInitiatorInit()) {         // Waits until the station is detected
      if (p2p.P2PInitiatorTxRx(0, 0, rx_buf, &rx_len)) { // Waits data
        flag = true;




  Serial.println(F("DEBUG: receive public key:"));
  for (int i = 0; i < rx_len; i++) {
    Serial.print(rx_buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();




        
      }
    }
  }
  
}











void setup() {

  Serial.begin (115200);            // Sets up serial port baudrate
  while (!Serial);                  // Waits until serial port is opened in PC

  RNG.begin (RNG_APP_TAG, RNG_SEED_ADDR); // Save a new seed for generating random numbers
  
  choose = serialInterface.introMenu ();  // Starts interactive menu and return user choose

}

void loop() {
    
  if ( choose == '1' ) {            // New game is going to start...

    // Erases information of previous events
    EEPROM.update (NUM_STATIONS_ADDR, 0); // Deletes the number of stations already setup

    // Generates a key pair for this event and save Secret Key in EEPROM
    Curve25519::dh1 (masterPk, masterSk); // Generates public and secret key for this event
    EEPROM.put (SK_ADDR, masterSk);       // Saves master secret key in EEPROM

    
    EEPROM.get (NUM_STATIONS_ADDR, stationID);  // Take the next station ID for setup
    setUpStations ();             // Start stations setup mode
    
  } else if ( (choose == '2') ) { // The setup of event is going to continue...

    EEPROM.get (NUM_STATIONS_ADDR, stationID);  // Take the next station for setup

    // Generates the master public key from EEPROM saved master secret key
    EEPROM.get (SK_ADDR, masterSk); // Load from EEPROM master secret key
    Curve25519::eval (masterPk, masterSk, 0); // Generates master public key
    
    setUpStations ();               // Start stations setup mode


  } else {
    choose = serialInterface.introMenu ();  // Start interactive menu and return user choose
  }

}
