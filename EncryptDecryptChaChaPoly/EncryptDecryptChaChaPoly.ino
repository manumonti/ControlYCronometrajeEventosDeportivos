/****************************************************************************/
/*
 * EncryptDecryptChaChaPoly
 * Created by Manuel Montenegro, January 8, 2017.
 * Developed for Manuel Montenegro Final Year Project. 
 * 
 *  This sketch encrypts and authenticates some data and, after that, decrypts
 *  and verifies the authenticated data. This encryption example comes from 
 *  RFC 7539 Internet Research Task Force (IRTF).
 *  
 *  Compatible boards with this sketch: Arduino UNO, Arduino Leonardo, Genuino
 *  101 and Intel Galileo.
*/
/****************************************************************************/

#include <ChaChaPoly.h>

// This object is used to manage ChaChaPoly functionalities
ChaChaPoly chachapoly;

// Key used for ChaChaPoly encryption and authentication (32 bytes)
uint8_t key [] = {  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
                          0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
                          0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
                          0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f };
                          
// Nonce (initialization vector) for ChaCha20 encryption
uint8_t iv [] = {   0x07, 0x00, 0x00, 0x00, 0x40, 0x41, 0x42, 0x43,
                          0x44, 0x45, 0x46, 0x47 };

// Data that will be authenticated but no encrypted
uint8_t aad [] = {  0x50, 0x51, 0x52, 0x53, 0xc0, 0xc1, 0xc2, 0xc3,
                          0xc4, 0xc5, 0xc6, 0xc7 };

// Plain text that will be authenticated and encrypted
uint8_t plainText [] = {  0x4c, 0x61, 0x64, 0x69, 0x65, 0x73, 0x20, 0x61,
                                0x6e, 0x64, 0x20, 0x47, 0x65, 0x6e, 0x74, 0x6c,
                                0x65, 0x6d, 0x65, 0x6e, 0x20, 0x6f, 0x66, 0x20,
                                0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x61, 0x73,
                                0x73, 0x20, 0x6f, 0x66, 0x20, 0x27, 0x39, 0x39,
                                0x3a, 0x20, 0x49, 0x66, 0x20, 0x49, 0x20, 0x63,
                                0x6f, 0x75, 0x6c, 0x64, 0x20, 0x6f, 0x66, 0x66,
                                0x65, 0x72, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x6f,
                                0x6e, 0x6c, 0x79, 0x20, 0x6f, 0x6e, 0x65, 0x20,
                                0x74, 0x69, 0x70, 0x20, 0x66, 0x6f, 0x72, 0x20,
                                0x74, 0x68, 0x65, 0x20, 0x66, 0x75, 0x74, 0x75,
                                0x72, 0x65, 0x2c, 0x20, 0x73, 0x75, 0x6e, 0x73,
                                0x63, 0x72, 0x65, 0x65, 0x6e, 0x20, 0x77, 0x6f,
                                0x75, 0x6c, 0x64, 0x20, 0x62, 0x65, 0x20, 0x69,
                                0x74, 0x2e };  


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
  
  // Opens serial port
  Serial.begin (9600);

  // Sketch doesn't start if no device listening (necessary for Arduino Leonardo)
  while (!Serial);

  // Flushes previous data in the serial port (necessary for Intel Galileo)
  Serial.println();


  // This array will save the encrypted plaintext
  uint8_t cipherText [sizeof(plainText)];

  // This array will save the encrypted plaintext
  uint8_t decipherText [sizeof(plainText)];

  // This array will save the authenticated tag
  uint8_t authTag [16];

  // For calculating elapsed time
  unsigned long timeStart;
  unsigned long timeEnd; 


  // Sets the key for ChaChaPoly
  chachapoly.setKey ( key, sizeof(key) );

  // Sets the Nonce (IV) for ChaChaPoly
  chachapoly.setIV ( iv, sizeof(iv) );
  
  // Adds the data that will be authenticated but no encrypted
  chachapoly.addAuthData ( aad, sizeof(aad) );

  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros ();

  // Encrypts the plain text
  chachapoly.encrypt ( cipherText, plainText, sizeof (plainText) );

  // Time at the end of the processs
  timeEnd = micros();

  Serial.print( "Encripting data elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();

  timeStart = micros ();

  // Calculates the Poly1305 authentication tag
  chachapoly.computeTag ( authTag, sizeof(authTag) );

  timeEnd = micros();

  Serial.print( "Computing authentication tag elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();


  Serial.println ( "Plain text:" );
  printHex ( plainText, sizeof(plainText) );
  
  Serial.println ( "Ciphered text:" );
  printHex ( cipherText, sizeof(cipherText) );

  Serial.println ( "Authentication tag:" );
  printHex ( authTag, sizeof(authTag) );


  // Clears all security-sensitive state from this cipher.
  chachapoly.clear();

  // Sets the key for ChaChaPoly
  chachapoly.setKey ( key, sizeof(key) );

  // Sets the Nonce (IV) for ChaChaPoly
  chachapoly.setIV ( iv, sizeof(iv) );

  // Adds the data that will be authenticated but no encrypted
  chachapoly.addAuthData ( aad, sizeof(aad) );

  // Decrypts the ciphered text
  chachapoly.decrypt ( decipherText, cipherText, sizeof(cipherText) );

  Serial.println ( "Deciphered text:" );
  printHex ( decipherText, sizeof(decipherText) ); 

  // Checks if Authentication Tag is correct
  if ( chachapoly.checkTag ( authTag, sizeof (authTag) ) ) {
    Serial.println ( "The authentication tag is correct" );
  } else {
    Serial.println ( "Incorrect authentication tag" );
  }  
  
}


void loop() {}
