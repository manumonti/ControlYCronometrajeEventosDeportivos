/*********************************************************************************************/
/*
 * MASTER
 * Created by Manuel Montenegro, January 18, 2017.
 * Developed for Manuel Montenegro Final Year Project. 
 * 
 *  This sketch sets up station devices by NFC. It assignes an identifier (ID) and a shared 
 *  key using Diffie-Hellman algorithm. Persistent data is saved in EEPROM.
 * 
 *  This sketch must be uploaded in master device. Serial connection to PC and NFC shield is 
 *  required. Serial port baudrate: 115200
 *  
 *  Compatible boards with this sketch: Arduino UNO, Arduino Leonardo.
*/
/*********************************************************************************************/

#include <RNG.h>                    // Random Number Generator library
#include <SHA256.h>                 // HMAC SHA256 library
#include <Curve25519.h>             // Diffie-Hellman library
#include <P2P-PN532.h>              // NFC P2P library
#include <EEPROM.h>                 // Arduino EEPROM management library
#include <RTClib.h>                 // Real Time Clock library
#include <AT24CX.h>                 // I2C EEPROM in RTC module management library
#include <SerialInterface.h>        // Interface with the user by serial port and PC library

#define I2C_EEPROM_ADDR     0x57    // I2C Address of EEPROM integrated in RTC module
#define KEY_SIZE            32      // Size in bytes of keys used
#define STATION_REC_SIZE    32      // Size in bytes of each station record
#define RX_BUF_MAX_SIZE     32      // Max size in bytes of message that MASTER can receive
#define TX_BUF_MAX_SIZE     49      // Max size in bytes of message that MASTER can send
#define CHALLENGE_SIZE      16      // Size in bytes of generated challenge
#define TIME_SIZE           4       // Size in bytes of clock time
#define NUM_STATIONS_ADDR   0       // EEPROM address where is saved the number of stations
#define RNG_SEED_ADDR       1       // EEPROM address where is saved the seed for RNG
#define SK_ADDR             50      // EEPROM address where is saved secret key of the event
#define RNG_APP_TAG         "master"// Name unique of this app for taking RNG seed

uint8_t choose;                     // User choose from serial menu
uint8_t stationID;                  // ID of current station
uint8_t challenge [CHALLENGE_SIZE]; // For stores the challenge
uint8_t masterPk [KEY_SIZE];        // Master Diffie-Hellman public key
uint8_t masterSk [KEY_SIZE];        // Master Diffie-Hellman secret key
uint8_t hmac [KEY_SIZE];            // HMAC received from station
uint8_t stationPk [KEY_SIZE];       // Station public key received

P2PPN532 p2p;                       // Object that manages NFC P2P connection
SerialInterface serialInterface;    // Object that manages the user interface
SHA256 sha256;                      // Object that manages HMAC & SHA256 functionalities
RTC_DS3231 rtc;                     // Object that manages Real Time Clock
AT24CX i2cEeprom;                   // Object that manages I2C EEPROM in RTC module

// Sets up stations until it receives a finish command.
void setUpStations ( ) {

  p2p.begin();                      // Configures & resets PN532 module
  p2p.SAMConfiguration();           // Configure the Secure Access Module of PN532 for P2P
  
  choose = serialInterface.setupMenu(stationID);  // Starts setup interactive menu

  while ( choose == '1' ) {         // If user chooses set up a new station...

    uint8_t flag;                   // Control flag

    sendP2PChallenge ();            // Sends challenge to the station
    receiveP2PResponse();           // Receives station public key and HMAC
    calculateSharedKey();           // Calculates keys of station & saves in I2C EEPROM
    flag = calculateAndCheckHMAC(); // Checks HMAC received
    
    if ( flag ) {
      // Ask for a new station or finish setup process 
      EEPROM [NUM_STATIONS_ADDR] += 1;  // Update the number of stations in Arduino EEPROM
      EEPROM.get (NUM_STATIONS_ADDR, stationID);  // Loads the next station number
      choose = serialInterface.setupMenu (stationID);
    } else {
      Serial.println (F("DEBUG: Error en HMAC"));
      choose = serialInterface.setupMenu (stationID);
    }
    
  }

  choose = 0;                       // Put the flag in "invalid option"
  
}


// Send a challenge randomly generated
void sendP2PChallenge () {
  
  uint8_t tx_buf [TX_BUF_MAX_SIZE]; // Buffer that will be sent
  uint32_t timeStamp;               // Buffer that will contains time stamp
  uint8_t randomNumber [CHALLENGE_SIZE - TIME_SIZE]; // Buffer for random generation
  uint8_t flag;                     // Control flag

  // Generates the challenge
  RNG.rand (randomNumber, sizeof(randomNumber));  // Random generation for challenge
  timeStamp = rtc.now().unixtime(); // Receives time from RTC the real time
  memcpy (challenge, &timeStamp, TIME_SIZE);  // Introduces time in challenge
  memcpy (&challenge[TIME_SIZE], randomNumber, sizeof(randomNumber)); //Introduces random

  // Makes the send buffer with all the information
  tx_buf[0] = stationID;            // Station identifier
  memcpy(&tx_buf[1], challenge, sizeof(challenge)); // Challenge
  memcpy(&tx_buf[17], masterPk, sizeof(masterPk));  // Master public key

  // Sends station ID, challenge and master public key
  flag = false;
  while (!flag) {
    if (p2p.P2PInitiatorInit()) {   // Waits until the station is detected
      if (p2p.P2PInitiatorTxRx(tx_buf, sizeof(tx_buf), 0, 0)) { // Sends data
        flag = true;
      }
    }
  }
}



// Receives station response to P2P Challenge
void receiveP2PResponse () {
  
  uint8_t rx_buf [RX_BUF_MAX_SIZE]; // Buffer that will be received
  uint8_t rx_len;                   // Size of data received
  uint8_t flag;                     // Control flag

  // Receives Station Public key
  flag = false;
  while (!flag) {
    if (p2p.P2PTargetInit()) {      // Waits until the station is detected
      if (p2p.P2PTargetTxRx(0, 0, rx_buf, &rx_len)) { // Waits data (public key)
        memcpy (stationPk, rx_buf, rx_len); // Copies station public key in memory
        flag = true;      
      }
    }
  }

  // Receives Station HMAC
  flag = false;
  while (!flag) {
    if (p2p.P2PTargetInit()) {      // Waits until the station is detected
      if (p2p.P2PTargetTxRx(0, 0, rx_buf, &rx_len)) { // Waits data (HMAC)
        memcpy (hmac, rx_buf, KEY_SIZE);  // Copies HMAC in memory
        flag = true;
      }
    }
  }
  
}

// Calculates the shared key of station and saves it in EEPROM
void calculateSharedKey() {

  uint8_t sharedKey [KEY_SIZE];     // Stores Diffie-Hellman shared key

  EEPROM.get (SK_ADDR, masterSk);   // Load from EEPROM master secret key
  memcpy (sharedKey, stationPk, sizeof(stationPk)); // Copies station public key.
  Curve25519::dh2 (sharedKey, masterSk);// Generates Diffie-Hellman shared key

  // Saves the station key on I2C EEPROM
  i2cEeprom.write (stationID*STATION_REC_SIZE, sharedKey, STATION_REC_SIZE); 
  
}

// Calculates HMAC of stationID, challenge & station public key & check with receivedHMAC
bool calculateAndCheckHMAC () {
  
  uint8_t calculatedHMAC [KEY_SIZE];// Stores calculated HMAC for checking
  uint8_t sharedKey [KEY_SIZE];     // Key of the station
  
  // Saves the station key on I2C EEPROM
  i2cEeprom.read(stationID*STATION_REC_SIZE, sharedKey, STATION_REC_SIZE);

            Serial.println(stationID,HEX);
            Serial.println(sizeof(stationID));
            Serial.println("DEBUG: challenge");
            for (int i = 0; i < sizeof(challenge); i++) {
              Serial.print (challenge[i], HEX);
              Serial.print (" ");
            }
            Serial.println();

            Serial.println("DEBUG: stationPk");
            for (int i = 0; i < sizeof(stationPk); i++) {
              Serial.print (stationPk[i], HEX);
              Serial.print (" ");
            }
            Serial.println();    
            
            Serial.println("DEBUG: shared key");
            for (int i = 0; i < sizeof(sharedKey); i++) {
              Serial.print (sharedKey[i], HEX);
              Serial.print (" ");
            }
            Serial.println();
   
  // Calculating the HMAC
  sha256.resetHMAC(sharedKey, sizeof(sharedKey)); // Inits HMAC process
  sha256.update(&stationID, sizeof(stationID));    // Introduces station ID
  sha256.update(challenge, sizeof(challenge));    // Introduces challenge
  sha256.update(stationPk, sizeof(stationPk));    // Introduces station public key
  sha256.finalizeHMAC(sharedKey, sizeof(sharedKey), calculatedHMAC, sizeof(calculatedHMAC));

            Serial.println("DEBUG: calculatedHMAC");
            for (int i = 0; i < sizeof(calculatedHMAC); i++) {
              Serial.print (calculatedHMAC[i], HEX);
              Serial.print (" ");
            }
            Serial.println();

  // Check hmac calculated and received is the same
  if (  memcmp (hmac, calculatedHMAC, sizeof(hmac)) == 0  ) {
    return true;
  } else {
    return false;
  }  
}








void setup() {

  Serial.begin (115200);            // Sets up serial port baudrate
  while (!Serial);                  // Waits until serial port is opened in PC

  RNG.begin (RNG_APP_TAG, RNG_SEED_ADDR); // Save a new seed for generating random numbers

  i2cEeprom = AT24C32(I2C_EEPROM_ADDR); // Inits I2C EEPROM in RTC module in I2C address
  rtc.begin();                      // Inits rtc object
  if (rtc.lostPower()) {            // If time is not adjusted
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));// Adjuts time that sketch was compilated
  }
  
  choose = serialInterface.introMenu ();  // Starts interactive menu and return user choose

}

void loop() {
    
  if ( choose == '1' ) {            // New game is going to start...

    // Erases information of previous events
    EEPROM.update (NUM_STATIONS_ADDR, 0); // Deletes the number of stations already setup

    // Generates a key pair for this event and save Secret Key in EEPROM
    Curve25519::dh1 (masterPk, masterSk); // Generates public and secret keys for this event
    EEPROM.put (SK_ADDR, masterSk);       // Saves master secret key in Arduino EEPROM

    EEPROM.get (NUM_STATIONS_ADDR, stationID);  // Take the next station ID for setup
    setUpStations ();               // Start stations setup mode
    
  } else if ( (choose == '2') ) {   // The setup of event is going to continue...

    EEPROM.get (NUM_STATIONS_ADDR, stationID);  // Take the next station for setup

    // Generates the master public key from EEPROM saved master secret key
    EEPROM.get (SK_ADDR, masterSk); // Load from Arduino EEPROM master secret key
    Curve25519::eval (masterPk, masterSk, 0); // Generates master public key
    
    setUpStations ();               // Start stations setup mode

  } else {
    choose = serialInterface.introMenu ();  // Start interactive menu and return user choose
  }

}
