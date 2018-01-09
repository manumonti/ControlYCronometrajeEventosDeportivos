/****************************************************************************/
/*
 * GenerateSHA256HMAC
 * Created by Manuel Montenegro, January 8, 2017.
 * Developed for Manuel Montenegro Final Year Project. 
 * 
 *  This sketch generates a SHA256 Cryptographic hash from a string and 
 *  print the elapsed time during processing.
 *  
 *  Compatible boards with this sketch: Arduino UNO, Arduino Leonardo, Genuino 
 *  101 and Intel Galileo.
*/
/****************************************************************************/

#include <SHA256.h>

// This object is used to manage SHA256 functionalities
SHA256 sha256;

// Data for calculating the HMAC
const char data [] = "Lorem ipsum dolor sit amet";

// Key used for HMAC
const char key [] = "key";


// Prints bytes array data in hex with leading zeroes
void printHex ( uint8_t data [], uint8_t length ) {
   for (int i=0; i<length; i++) {
    if (data[i]<0x10) {
      Serial.print("0");
    }
    Serial.print(data[i],HEX);
    Serial.print(" ");
   }
   Serial.println();
}


void setup() {
  // Open serial port
  Serial.begin(9600);

  // Don't start sketch if no device listening (necessary for Arduino Leonardo)
  while (!Serial);

  // Flush previous data in the serial port (necessary for Intel Galileo)
  Serial.println();

  // Array where will be placed the HMAC
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
  printHex ( result, sizeof(result) );

  //  Printing the elapsed time
  Serial.println();
  Serial.print("Elapsed time during ciphering (in microseconds): ");
  Serial.println(timeEnd-timeStart);

}

void loop() {

}
