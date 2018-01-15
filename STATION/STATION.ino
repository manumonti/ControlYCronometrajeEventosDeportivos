/*********************************************************************************************/
/*
 * STATION
 * Created by Manuel Montenegro, January 15, 2017.
 * Developed for Manuel Montenegro Final Year Project. 
 * 
 *  This sketch manages each station of the platform.
 * 
 *  Serial port is only for debug. Serial port baudrate: 115200
 *  
 *  Compatible boards with this sketch: Arduino UNO.
*/
/*********************************************************************************************/

#include <RNG.h>                    // Random Number Generator library
#include <SHA256.h>                 // HMAC SHA256 library
#include <Curve25519.h>             // Diffie-Hellman library
#include <P2P-PN532.h>              // NFC P2P library
#include <EEPROM.h>                 // EEPROM management library

P2PPN532 p2p;                       // Object that manages NFC P2P connection
SHA256 sha256;                      // Object that manages HMAC & SHA256 functionalities

#define KEY_SIZE            32      // Size in bytes of keys used
#define STATION_ID_ADDR     0       // EEPROM address where is saved the station identifier
#define RNG_SEED_ADDR       1       // EEPROM address where is saved the seed for RNG
#define SHARED_KEY_ADDR     82      // EEPROM address where begins stations records
#define RX_BUF_MAX_SIZE     17      // Max size in bytes of message that MASTER can receive
#define RNG_APP_TAG         "station" // Name unique of this app for taking RNG seed

uint8_t stationID;                  // ID of current station
uint8_t challenge [16];             // Array for generating the challenge
uint8_t stationPk [KEY_SIZE];       // Station Diffie-Hellman public key
uint8_t stationSk_HMAC [KEY_SIZE];  // Station Diffie-Hellman secret key
uint8_t masterPk_Shared [KEY_SIZE]; // Can have two values: Master public key & shared key

uint8_t rx_buf [RX_BUF_MAX_SIZE];   // Buffer that will be received
uint8_t rx_len;                     // Size of data received

uint8_t flag;                       // Control flag for multiples purpouses


// Sets up the station: assignes crypto keys, station identifier, etc.
void setUpStation () {
  
  RNG.begin (RNG_APP_TAG, RNG_SEED_ADDR); // Save a new seed for generating random numbers

  p2p.begin();                      // Configures & resets PN532 module
  p2p.SAMConfiguration();           // Configure the Secure Access Module of PN532 for P2P

  Curve25519::dh1 (stationPk, stationSk_HMAC); // Gen public-secret keys for this station
  
}


// Starts NFC P2P communication with Master for setting up the station
void startP2PCommunication () {

  stationID = rx_buf[0];          // Station identifier
  memcpy (challenge, rx_buf[1], sizeof(challenge));  // Challenge
  memcpy (masterPk_Shared, rx_buf[17], sizeof(masterPk_Shared));  // Master public key

  Curve25519::dh2 (masterPk_Shared, stationSk_HMAC); // Generates Diffie-Hellman shared key
  EEPROM.put (STATION_ID_ADDR, stationID);          // Saves in EEPROM the station ID
  EEPROM.put (SHARED_KEY_ADDR, masterPk_Shared);    // Saves in EEPROM the shared key

  stationID = rx_buf[0];          // Station identifier

  Serial.println(F("DEBUG: Shared key: "));
  for (int i = 0; i < sizeof(masterPk_Shared); i++) {
    Serial.print(masterPk_Shared[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  sendHMACsetup();                    // Calculates HMAC and stores it in stationSk_HMAC

  Serial.println(F("Se va a enviar la respuesta..."));
  sendP2P();                      // Sends HMAC & Station public key to master
    
}

// Calculates HMAC of stationID, challenge & station public key & stores it in stationSk_HMAC
void sendHMACsetup () {
  
  // Calculating the HMAC. Var stationSk_HMAC is reused because it is empty after DH2 function
  sha256.resetHMAC(masterPk_Shared, sizeof(masterPk_Shared)); // Inits HMAC process
  sha256.update(stationID, sizeof(stationID));                // Introduces station ID
  sha256.update(challenge, sizeof(challenge));                // Introduces challenge
  sha256.update(stationPk, sizeof(stationPk));                // Introduces station public key
  sha256.finalizeHMAC(masterPk_Shared, sizeof(masterPk_Shared), stationSk_HMAC, sizeof(stationSk_HMAC));

  Serial.println(F("DEBUG: HMAC: "));
    for (int i = 0; i < sizeof(stationSk_HMAC); i++) {
      Serial.print(stationSk_HMAC[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
}

void sendP2P () {

  flag = false;
  while (!flag) {
    if(p2p.P2PTargetInit()){
      Serial.println(F("DEBUG: SECOND: Initiator is sensed."));
      if(p2p.P2PTargetTxRx(stationPk, sizeof(stationPk), rx_buf, &rx_len)){  
        flag = true;
      }
    }
  }

  flag = false;
  while (!flag) {
    //SEGUIR POR AQUÍ ----------------------------------------------------------------------------------------------------
  }
  
}






void setup() {
  Serial.begin (115200);            // Sets up serial port baudrate
  while (!Serial);                  // Waits until serial port is opened in PC

  setUpStation();                   // Init setup mode

}

void loop() {

  // Receives challenge message. Doesn't send anything
  if(p2p.P2PTargetInit()){
    Serial.println(F("DEBUG: FIRST: Initiator is sensed."));
    if(p2p.P2PTargetTxRx(0, 0, rx_buf, &rx_len)){  
      startP2PCommunication ();
    }
  }

}
