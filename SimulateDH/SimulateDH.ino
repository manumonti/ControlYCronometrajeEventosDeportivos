/****************************************************************************/
/*
 * SimulateDH
 * Created by Manuel Montenegro, January 7, 2017.
 * Developed for Manuel Montenegro Final Year Project.
 * 
 *  This sketch simulates a Diffie-Hellman key exchange between two devices
 *  and prints the elapsed time during processing.
 *  
 *  Compatible boards with this sketch: Arduino UNO, Arduino Leonardo and 
 *  Intel Galileo.
*/
/****************************************************************************/

#include <Curve25519.h>
#include <RNG.h>

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
  Serial.begin (9600);

  // Don't start sketch if no device listening (necessary for Arduino Leonardo)
  while (!Serial);

  // Save a new seed for generating non-repeated random numbers
  RNG.begin("SimulateDHArduinoUno", 950);

  // Flush previous data in the serial port (necessary for Intel Galileo)
  Serial.println();

  // Arrays for Diffie-Hellman keys
  uint8_t alice_k [32];
  uint8_t alice_f [32];
  uint8_t bob_k [32];
  uint8_t bob_f [32];

  // For calculating elapsed time
  unsigned long timeStart;
  unsigned long timeEnd;
  

  // Alice k/f generation ....................................................
  Serial.println("Generating random k/f for Alice ...");

  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros();
  
  // Generate a 'f' (private) and a 'k' (public) value for Alice
  Curve25519::dh1 (alice_k, alice_f);

  // Time at the end of the process
  timeEnd = micros();

  Serial.print( "Alice k: " );
  printHex ( alice_k, sizeof (alice_k) );
  Serial.print( "Alice f: " );
  printHex ( alice_f, sizeof (alice_f) );
  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();


  // Bob  k/f generation .....................................................
  Serial.println("Generating random k/f for Bob ...");

  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros();

  // Generate a 'f' (private) and a 'k' (public) value for Bob.
  Curve25519::dh1 (bob_k, bob_f);

  // Time at the end of the process
  timeEnd = micros();

  Serial.print( "Bob k: " );
  printHex ( bob_k, sizeof (bob_k) );
  Serial.print( "Bob f: " );
  printHex ( bob_f, sizeof (bob_f) );
  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();


  // Alice shared secret generation .........................................
  Serial.println ("Generating shared secret for Alice ...");
  
  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros();

  // Generate shared secret for Alice and destroy the private key
  Curve25519::dh2(bob_k, alice_f);

  // Time at the end of the process
  timeEnd = micros();

  Serial.print( "Alice shared key: " );
  printHex ( bob_k, sizeof (bob_k) );

  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();


  // Bob shared secret generation ...........................................
  Serial.println ("Generating shared secret for Bob ...");
  
  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros();

  // Generate shared secret for Alice and destroy the private key
  Curve25519::dh2(alice_k, bob_f);

  // Time at the end of the process
  timeEnd = micros();

  Serial.print( "Bob shared key: " );
  printHex ( alice_k, sizeof (alice_k) );

  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();
}

void loop() {
}
