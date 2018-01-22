/*********************************************************************************************/
/*
 * MASTER
 * Created by Manuel Montenegro, January 21, 2017.
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

#include <SerialInterface.h>        // Interface with the user by serial port and PC library
#include <SetUpStations.h>          // Stations' setup library

uint8_t userChoose;                 // User choose from serial menu

SerialInterface serialInterface;    // Manages the user's interface

void setup() {

  Serial.begin (115200);            // Sets up serial port baudrate
  while (!Serial);                  // Waits until serial port is opened in PC

}

void loop() {
  
  userChoose = serialInterface.introMenu ();  // Start interactive menu

  if (userChoose == '1') {
    MasterSetUpStations setUp;      // Manages the stations' setup
    setUp.startNewEvent ();         // Starts the process of setting up new stations
  } else if (userChoose == '2') {
    MasterSetUpStations setUp;      // Manages the stations' setup
    setUp.continuePreviousEvent (); // Continues a previous process of setting up
  }


}
