/****************************************************************************/
/*
 * SimulateDHArduinoUno
 * Created by Manuel Montenegro, December 27, 2017.
 * Developed by Manuel Montenegro Final Year Project.
 * 
 *  This sketch simulates a Diffie-Hellman key exchange between two devices
 *  and prints the elapsed time during processing.
 *  
 *  Compatible boards with this sketch: Arduino UNO and Intel Galileo.
*/
/****************************************************************************/

#include <Curve25519.h>
#include <RNG.h>

// Print a generated key in hexadecimal
void printKey ( uint8_t key [32] ) {
  for (int i = 0; i < 32; i++) {
    Serial.print (key [i], HEX);
  }
  Serial.println();
}

void setup() {

  // Save a new seed for generating non-repeated random numbers
  RNG.begin("SimulateDHArduinoUno", 950);
  
  Serial.begin (9600);

  // Flush previous data in the serial port
  Serial.println();

  // Arrays for Diffie-Hellman keys
  uint8_t alice_k [32];
  uint8_t alice_f [32];
  uint8_t bob_k [32];
  uint8_t bob_f [32];

  // For calculating elapsed time
  unsigned long timeStart;
  unsigned long timeEnd;
  

  // Alice k/f generation .....................................................
  Serial.println("Generating random k/f for Alice ...");

  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros();
  
  // Generate a 'f' (private) and a 'k' (public) value for Alice
  Curve25519::dh1 (alice_k, alice_f);

  // Time at the end of the process
  timeEnd = micros();

  Serial.print( "Alice k: " );
  printKey (alice_k);
  Serial.print( "Alice f: " );
  printKey (alice_f);
  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();


  // Bob  k/f generation ......................................................
  Serial.println("Generating random k/f for Bob ...");

  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros();

  // Generate a 'f' (private) and a 'k' (public) value for Bob.
  Curve25519::dh1 (bob_k, bob_f);

  // Time at the end of the process
  timeEnd = micros();

  Serial.print( "Bob k: " );
  printKey (bob_k);
  Serial.print( "Bob f: " );
  printKey (bob_f);
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
  printKey (bob_k);

  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();


  // Bob shared secret generation .........................................
  Serial.println ("Generating shared secret for Bob ...");
  
  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros();

  // Generate shared secret for Alice and destroy the private key
  Curve25519::dh2(alice_k, bob_f);

  // Time at the end of the process
  timeEnd = micros();

  Serial.print( "Bob shared key: " );
  printKey (alice_k);

  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();
}

void loop() {
}
