/*********************************************************************************************/
/*
 * STATION_NBIOT
 * Created by Manuel Montenegro, March 01, 2017.
 * Developed for Manuel Montenegro Bachelor Thesis
 * 
 *  This sketch manages each station of the platform and use NB-IoT Sodaq Shield with ublox
 *  N211 for communication by NB-IoT. This connection is used for publish temporary results of
 *  a sport event.
 * 
 *  Serial port "Serial" is for USB debug. Serial port "Serial1" stablish a serial 
 *  communication with NB-IoT modem ublox N211.
 *  
 *  Compatible boards with this sketch: Arduino Leonardo.
*/
/*********************************************************************************************/

#include <SetUpStations.h>          // Stations' setup library
#include <PlayerCard.h>             // User's card management library
#include <SodaqNBIoT.h>             // NBIoT trough Sodaq NB-IoT Shield management library

#define LED_PIN           3         // Digital Pin where is tied LED
#define CARD_TIMEOUT      1         // Number of seconds between punch
#define MIFARE_BLOCK_SIZE 16        // Size of each block on Mifare Classic 1k Card

String SERVER_IP = "79.114.88.15";  // IP of UDP server
String SERVER_PORT = "16666";       // Port of UDP server

PlayerCard card;                    // Manages operation with user cards
SodaqNBIoT nbiot;                   // Ublox module

int socket;                         // Socket for sending & receiving data
uint8_t data [MIFARE_BLOCK_SIZE];               
uint8_t idUser [7];

void setup() {

  // Start ublox modem
  if ( nbiot.begin() ) {            // If device is registered in network
    socket = nbiot.openSocket (10000);
  }
    
  pinMode(LED_PIN, OUTPUT);         // Set Up digital pin for LED

  StationNewSetUp stationSetUp;     // Manages the stations' setup

  digitalWrite(LED_PIN, HIGH);      // Turn on LED for indicating set up period has started

  stationSetUp.startNewSetUp ();    // Starts the process of setting up station

  digitalWrite (LED_PIN, LOW);      // Turn off LED for indicating set up period has finished

  card.begin();
  
}

void loop() {

  if ( card.punch(data, idUser) ) {
  
    digitalWrite (LED_PIN, HIGH);
    delay (50);
    digitalWrite (LED_PIN, LOW);
    delay(50);
  
    nbiot.sendPunch (data, idUser, socket, SERVER_IP, SERVER_PORT);
  
    delay(CARD_TIMEOUT*1000);

  }
  
}
