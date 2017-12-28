/****************************************************************************/
/*
 * GenerateSHA256ArduinoUNO
 * Created by Manuel Montenegro, December 27, 2017.
 * Developed by Manuel Montenegro for Final Year Project. 
 * 
 *  This sketch generates a SHA256 Cryptographic hash from a string and 
 *  print the elapsed time during processing.
 *  
 *  Compatible boards with this sketch: Arduino UNO and Intel Galileo.
*/
/****************************************************************************/

#include <SHA256.h>

// This object is used to manage SHA256 functionalities
SHA256 sha256;

// Data for calculating the HMAC
const char data [] = "Lorem ipsum dolor sit amet";

// Key used for HMAC
const char key [] = "key";

void setup() {
  Serial.begin(9600);

  // Flush previous data in the serial port
  Serial.println();

  // Array where is going to be placed the HMAC
  uint8_t result [sha256.hashSize()];

  // Time at the beginning of the ciphering. For calculating the elapsed time
  unsigned long timeStart = micros();

  // Calculating the HMAC...
  sha256.resetHMAC(key, strlen(key));
  sha256.update(data, strlen(data));
  sha256.finalizeHMAC(key, strlen(key), result, sizeof(result));

  // Time at the end of ciphering.
  unsigned long timeEnd = micros();

  // Printing the information...
  Serial.print ("Key: ");
  Serial.println (key);
  Serial.print ("Data: ");
  Serial.println (data);
  Serial.println ();

  // Printing the calculated HMAC
  for (int i = 0; i < sizeof(result); i++) {
    Serial.print (result[i], HEX);
  }

  //  Printing the elapsed time
  Serial.println();
  Serial.print("Elapsed time during ciphering (in microseconds): ");
  Serial.println(timeEnd-timeStart);

}

void loop() {

}
