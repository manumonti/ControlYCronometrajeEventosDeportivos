/*********************************************************************************************/
/*
 * STATION
 * Created by Manuel Montenegro, January 26, 2017.
 * Developed for Manuel Montenegro Bachelor Thesis. 
 * 
 *  This sketch manages each station of the platform.
 * 
 *  Serial port is only for debug. Serial port baudrate: 115200
 *  
 *  Compatible boards with this sketch: Arduino UNO.
*/
/*********************************************************************************************/

#include <SetUpStations.h>          // Stations' setup library

void setup() {
  
  Serial.begin (115200);            // Sets up serial port baudrate (ONLY FOR DEBUG)

  StationNewSetUp stationSetUp;     // Manages the stations' setup
  stationSetUp.startNewSetUp ();    // Starts the process of setting up station
  
}

void loop() {
  
}
