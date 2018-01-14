/*********************************************************************************************/
/*
 * MASTER
 * Created by Manuel Montenegro, January 14, 2017.
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

#include <Curve25519.h>             // Diffie-Hellman library
#include <RNG.h>                    // Random Number Generator library
#include <EEPROM.h>                 // EEPROM management library
#include <P2P-PN532.h>              // NFC P2P library
#include <PN532.h>                  // NFC library
#include <SerialInterface.h>        // Interface with the user by serial port and PC library

#define NUM_STATIONS_ADDR   0       // EEPROM address where is saved the number of stations
#define RNG_SEED_ADDR       1       // EEPROM address where is saved the seed for RNG
#define FIRST_REC_ADDR      50      // EEPROM address where begins stations records
#define STATION_REC_SIZE    5       // Size in bytes of each station record
#define RNG_APP_TAG         "master"// Name unique of this app for taking RNG seed

P2PPN532 p2p;                       // Object that manages NFC P2P connection
SerialInterface serialInterface;    // Object that manages the user interface

uint8_t choose;                     // User choose from serial menu
uint8_t stationID;                  // ID of current station
uint8_t challenge [4];              // Array for generating the challenge


// Sets up stations until it receives a finish command.
void setUpStations ( ) {

  p2p.begin();                      // Configures & resets PN532 module
  p2p.SAMConfiguration();           // Configure the Secure Access Module of PN532 for P2P
  
  choose = serialInterface.setupMenu(stationID);  // Starts setup interactive menu

  while ( choose == '1' ) {

    RNG.rand (challenge, sizeof(challenge));  // Generates the challenge for this station

    uint8_t sended = false;
    while (!sended) {
      delay (100);                  // Waits PN532 is ready
      sended = sendChallenge ();    // Send challenge to the station and chech ACK
      Serial.println(sended);
    }
    
    uint8_t EEPROMaddr = FIRST_REC_ADDR + (STATION_REC_SIZE * stationID); // Address of this station
    EEPROM.update (EEPROMaddr, stationID);  // Writes the station ID in EEPROM
    EEPROM.put (EEPROMaddr+1, challenge);   // Writes the challenge of this station in EEPROM

    // Ask for a new station or finish setup process 
    EEPROM [NUM_STATIONS_ADDR] += 1;
    EEPROM.get (NUM_STATIONS_ADDR, stationID);
    choose = serialInterface.setupMenu (stationID);
    
  }

  choose = 0;                       // Put the flag in "invalid option"
  
}

bool sendChallenge () {
  uint8_t tx_buf [STATION_REC_SIZE];      // Buffer that will be sent
  uint8_t rx_buf [50];                    // Buffer that will be received
  uint8_t rx_len;                         // Size of data received

  // Make the buffer with station information
  rx_buf[0] = stationID;                  
  memcpy(&rx_buf[1], challenge, sizeof(challenge));

  uint8_t ok = false;
  while (!ok) {
    if (p2p.P2PInitiatorInit()) {         // Waits until the station is detected
      if (p2p.P2PInitiatorTxRx(tx_buf, sizeof(tx_buf), rx_buf, &rx_len)) { // Waits data
        ok = true;
      }
    }
  }
  
  if ( (rx_len == 1) && (rx_buf [0] == true) ) {  // Check ACK
    return true;
  } else {                       
    return false;
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

    EEPROM.get (NUM_STATIONS_ADDR, stationID);  // Take the next station for setup
    setUpStations ();             // Start stations setup mode
    
  } else if ( (choose == '2') ) { // The setup of event is going to continue...

    EEPROM.get (NUM_STATIONS_ADDR, stationID);  // Take the next station for setup
    setUpStations ();               // Start stations setup mode

  } else {
    choose = serialInterface.introMenu ();  // Start interactive menu and return user choose
  }

}
