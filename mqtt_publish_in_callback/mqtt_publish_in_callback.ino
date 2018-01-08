/*
 Publishing in the callback

  - connects to an MQTT server
  - subscribes to the topic "inTopic"
  - when a message is received, republishes it to "outTopic"

  This example shows how to publish messages within the
  callback function. The callback function header needs to
  be declared before the PubSubClient constructor and the
  actual callback defined afterwards.
  This ensures the client reference in the callback function
  is valid.

*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SHA256.h>
#include <Curve25519.h>
#include <RNG.h>



// Print a generated key in hexadecimal
//void printKey ( uint8_t key [32] ) {
//  for (int i = 0; i < 32; i++) {
//    Serial.print(i);
//    Serial.print("-");
//    Serial.print (key [i], HEX);
//  }
//  Serial.println();
//}

void printKey ( uint8_t key [32] ) {
 char tmp[64];
 bytesToHex(key,32,tmp );
 Serial.println(tmp);
}

void PrintHex8(uint8_t *data, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
     char tmp[16];
       for (int i=0; i<length; i++) { 
         sprintf(tmp, "0x%.2X,",data[i]); 
         //sprintf(tmp, "%.2X",data[i]); 
         Serial.print(tmp);
         Serial.print(" ");
       }
       Serial.println();
}

void bytesToHex(uint8_t *data, uint8_t length,char tmp[]) // prints 8-bit data in hex
{
 //char tmp[length*2+1];
 byte first ;
 int j=0;
 for (uint8_t i=0; i<length; i++) 
 {
   first = (data[i] >> 4) | 48;
   if (first > 57) tmp[j] = first + (byte)39;
   else tmp[j] = first ;
   j++;

   first = (data[i] & 0x0F) | 48;
   if (first > 57) tmp[j] = first + (byte)39; 
   else tmp[j] = first;
   j++;
 }
 tmp[length*2] = 0;
 // Serial.println(tmp);
}


// Update these with values suitable for your network.
//byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };

IPAddress server(54, 154, 205, 240); //broker.shiftr.io

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);


// This object is used to manage SHA256 functionalities
SHA256 sha256;

// Key used for HMAC
const char key [] = "key";

// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.
 Serial.print("Mensaje recibido en el canal "); Serial.println(topic);
 if (strcmp(topic,"challenge")==0)
 {
   Serial.println("Hemos recibido un PAYLOAD para hacer un HASH ");
   uint8_t result [sha256.hashSize()];
   sha256.resetHMAC(key, strlen(key));
   sha256.update(payload, length);
   sha256.finalizeHMAC(key, strlen(key), result, sizeof(result));
     // Printing the calculated HMAC
  for (int i = 0; i < sizeof(result); i++) {
    Serial.print (result[i], HEX);
  }
  Serial.println();
 
  // Allocate the correct amount of memory for the payload copy
  byte* p = (byte*)malloc(length);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  client.publish("hash/PEPE", p, length);
  // Free the memory
  free(p);
 }
}

void setup()
{

  //Ethernet.begin(mac, ip);
  
  system("ifconfig eth0 192.168.2.22 netmask 255.255.255.0 up");
  system("route add default gw 192.168.2.1 eth0");
  system("echo \"nameserver 8.8.8.8\" > /etc/resolv.conf");

  // Save a new seed for generating non-repeated random numbers
  RNG.begin("SimulateDHArduinoUno", 950);
  
  Serial.begin(9600);
  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();


  uint8_t pk1 [32] = { 0xA6, 0x21, 0x0F, 0x6A, 0x4C, 0xD2, 0x8A, 0x83, 0x98, 0x6C, 0xB2, 0x28, 0x4D, 0x4A, 0xA3, 0xE4, 0x19, 0x86, 0xEE, 0xF1, 0x57, 0xBC, 0xF7, 0xE3, 0xB9, 0xE0, 0x9C, 0x8B, 0xFE, 0x8F, 0x2B, 0x63 };
  uint8_t pk2 [32] = { 0x0E, 0x41, 0xC8, 0xCE, 0x8A, 0xFF, 0x43, 0xBE, 0x66, 0xB9, 0x53, 0x8F, 0x99, 0x44, 0x5A, 0xEA, 0xE1, 0x77, 0x8A, 0x03, 0xE5, 0x3C, 0x7D, 0x45, 0x84, 0xC8, 0x3C, 0x85, 0xE2, 0x7A, 0x35, 0x08 };

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


  Serial.print( "Alice Public (k): " );
  printKey (alice_k);
  PrintHex8 (alice_k,32);

  char pub_key[64];
  bytesToHex(alice_k,32,pub_key);
  
  Serial.print( "Alice Private (f): " );
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

  Serial.print( "Bob Public (K): " );
  printKey (bob_k);
  PrintHex8 (bob_k,32);
  Serial.print( "Bob Private (f): " );
  printKey (bob_f);
  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();


  // Alice shared secret generation .........................................
  Serial.println ("Generating shared secret for Alice ...");
  
  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros();

  // Generate shared secret for Alice and destroy the private key
  Curve25519::dh2(pk1, alice_f);

  // Time at the end of the process
  timeEnd = micros();

  Serial.print( "NODE shared key: " );
  printKey (pk1);

  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();


  // Bob shared secret generation .........................................
  Serial.println ("Generating shared secret for Bob ...");
  
  // Time at the beginning of the process. For calculating the elapsed time
  timeStart = micros();

  // Generate shared secret for Alice and destroy the private key
  //Curve25519::dh2(alice_k, bob_f);
  Curve25519::dh2(alice_k, bob_f);

  // Time at the end of the process
  timeEnd = micros();

  Serial.print( "Bob shared key: " );
  printKey (alice_k);

  Serial.print( "Elapsed time (in microseconds): " );
  Serial.println( timeEnd - timeStart );
  Serial.println();
  
  if (client.connect("arduinoClient2","try","try")) {
     Serial.println("Conectado");
    client.publish("outTopic","hello world");
    client.publish("public_dh/PEPE",pub_key);
    //client.subscribe("hash");
    client.subscribe("challenge");
  }
}

void loop()
{
  client.loop();
}
