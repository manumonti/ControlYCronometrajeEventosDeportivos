/*********************************************************************************************/
/*
 * ReadNFCPublishMQTT
 * Created by Manuel Montenegro, January 8, 2017.
 * Developed for Manuel Montenegro Final Year Project. 
 * 
 *  This sketch send some data readed from a Mifare Classic 1k NFC card when this is placed 
 *  on the reader.
 *  
 *  Compatible boards with this sketch: Arduino UNO and Genuino 101.
*/
/*********************************************************************************************/

#include <Ethernet.h>
#include <MQTTClient.h>
#include <Adafruit_PN532_mod.h> // TODO: change this library for self-made library

// In Adafruit PN532 Shield, IRQ pin is attached to digital pin 2
#define PN532_IRQ       2
#define PN532_RESET     3    // Not connected by default on the Adafruit NFC Shield

#define READ_TIMEOUT    7500 // Time that the NFC reader waits for a card (millisec.)
#define REREAD_TIMEOUT  5000 // Guard interval for avoiding read the card several times

// Each authentication key type has a ID number.
#define keyAType        0    //TODO: this should be in library
#define keyBType        1 

Adafruit_PN532 nfc (PN532_IRQ, PN532_RESET);  // Object managing PN532 module
EthernetClient net;                           // Object managing ethernet connection
MQTTClient mqttClient;                        // Object managing MQTT protocol

bool iso14443aCard;           // Flag to check if the card is an ISO 14443A card
uint8_t uid[7];               // Buffer to store the returned UID
uint8_t uidLength;            // Length of the UID (depending on card type)
uint8_t currentblock = 4;     // Counter to keep track of which block we're on
uint8_t data[16];             // Array to store block data during reads
char dataHex[33];             // Array to store block data in hexadecimal string

// Keyb of Mifare Classic NFC card of the proyect
uint8_t keyb[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// MAC and IP address of the network card
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {192, 168, 2, 2};


void messageReceived (String &topic, String &payload) {
  
}

// Connects to Shiftr.io platform using the token of the device
void mqttConnect () {
    Serial.println("Connecting to Shiftr.io platform...");
    while (!mqttClient.connect("Arduino", "0785c4b1", "37bd163a173b6c24")) {
    delay(250);
  }
  Serial.println("Connection stablished!");
}

// Converts a byte array to a string of the bytes values formatted in hexadecimal
// Buffer must be an array with length (2 * length of array + 1)
void bytesToHexString(byte array[], unsigned int len, char buffer[]) {
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}


void setup() {

  // Setting serial port baudrate
  Serial.begin (115200);

  // Wait until serial port is stablished (necessary for Genuino 101)
  while (!Serial);

  nfc.begin();   // I2C initialization & resets PN532 module
  
  // Check if PN532 is correctly connected to Arduino pins
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.println("Didn't find PN532 board");
    while (1); // halt
  }

  nfc.SAMConfig();  // Configure the Secure Access Module of PN532

  Ethernet.begin(mac, ip);                    // Ethernet connection initialization
  mqttClient.begin("broker.shiftr.io", net);  // MQTT client initialization
  mqttConnect ();                             // Connection to shiftr.io namespace
  
}

void loop() {

  // Send messages and check for incoming messages from MQTT
  mqttClient.loop();

  // Check if connection is still stablished
  if (!mqttClient.connected()) {
    mqttConnect();
  }

  // Wait until a valid card is placed on the reader or elapse a timeout
  // If the sketch blocks more than 15 seconds here, Shiftr.io will disconnect the device
  iso14443aCard = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, READ_TIMEOUT);

  // If the UID has been readed correctly and the card type is valid...
  // (Mifare Classic 1K has a UID length of 4 bytes)
  if ( iso14443aCard && (uidLength == 4) ) {
    
    //Authenticates block sector that is going to be readed
    if ( nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, keyBType, keyb) ) {

      // Reads the entire block
      if ( nfc.mifareclassic_ReadDataBlock(currentblock, data) ) {
        
        // formats the byte array to a string of the bytes values expressed in hex
        bytesToHexString (data, sizeof(data), dataHex);
        // sends the block readed
        mqttClient.publish("/Stations/Station1", dataHex);
        
      }
    }
  }
}

