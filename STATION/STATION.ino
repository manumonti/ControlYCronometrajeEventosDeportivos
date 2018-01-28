/*********************************************************************************************/
/*
 * STATION
 * Created by Manuel Montenegro, January 28, 2017.
 * Developed for Manuel Montenegro Bachelor Thesis. 
 * 
 *  This sketch manages each station of the platform.
 * 
 *  Serial port is only for debug. Serial port baudrate: 115200
 *  
 *  Compatible boards with this sketch: Arduino UNO and Arduino Leonardo.
*/
/*********************************************************************************************/

#include <SetUpStations.h>          // Stations' setup library
#include <PlayerCard.h>             // User's card management library

#include <RTClib.h>

#define LED_PIN           9         // Digital Pin where is tied LED

PlayerCard card;                    // Manages operation with user cards


void setup() {
  
  Serial.begin (115200);            // Sets up serial port baudrate (ONLY FOR DEBUG)  
  
  pinMode(LED_PIN, OUTPUT);         // Set Up digital pin for LED
  digitalWrite(LED_PIN, HIGH);      // Turn on LED for indicating set up period has started

  StationNewSetUp stationSetUp;     // Manages the stations' setup
  stationSetUp.startNewSetUp ();    // Starts the process of setting up station

  card.begin();

  digitalWrite (LED_PIN, LOW);      // Turn off LED for indicating set up period has finished

  RTC_DS3231 rtc; Serial.println(rtc.now().unixtime());
}

void loop() {



  Serial.println("Puedes hacer punch");
  card.punch();
  Serial.println("fin de punch");
  
  digitalWrite (LED_PIN, HIGH);
  delay (100);
  digitalWrite (LED_PIN, LOW);
  delay(100);
  digitalWrite (LED_PIN, HIGH);
  delay (100);
  digitalWrite (LED_PIN, LOW);

  delay(100000);
  
}
