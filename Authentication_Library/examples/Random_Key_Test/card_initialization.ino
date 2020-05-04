/*------------------( Include Libraries )-------------------------------*/
#include "RTClib.h"                     // For use with DS3231 to keep time when disconnected includes Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>                       // Used in serial communication
#include <SPI.h>                        // Used in serial communication
#include <EEPROM.h>                     // For persistent storage of data when disconnected (future charge expiration time, verified key ids, etc.)
#include <Adafruit_PN532.h>             // For use with a PN532 to interact with RFID cards
#include <Authentication_Library.h>

/*---------------( Declare Constants and Pin Numbers )-----------------*/
// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MISO (3)
#define PN532_MOSI (4)
#define PN532_SS   (5)

uint8_t success;                          // defines a variable to check the success of an NFC card scan
String cardRecord;                        // defines a variable to check the success of an NFC record scan
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID after NFC card scan
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type) for NFC card scan
uint16_t timeout = 5000;                  // defines a variable to timeout the card reader function, in ms
char rx_byte = 0;                         // stores input from serial monitor
//uint8_t record[32];

/*-------------------------( Declare objects )--------------------------*/
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS); // Create a nfc object for a breakout with a software SPI connection

// For the purpose of random number generation we use analog readings from analog pins 0-3 and 6-7.
// They should remain unplugged or the library should be modified to adapt.

KeyDatabase keyDB;
void setup() {
 Serial.begin(9600);
 delay(3000);                  // wait for console opening
 nfc.begin();
 uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata)
  {
    Serial.print("Didn't find PN53x board");
    while (1);                              // I think we're going to eventually do something different, I don't want the outlet to fail if the NFC reader fails
  }
  //  nfc.setPassiveActivationRetries(0xFF);  // Sets the maximum number of retries.  0xFF retries forever.  Currently using a timeout function in success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeout);
  nfc.SAMConfig();                          // configure board to read RFID tags
}

void loop() {
  rx_byte = 0;
  Serial.println("Enter A to program Admin card, or Enter C to program Commuter card");
  while(!Serial.available()){}
  if (Serial.available() > 0) {    // is a character available?
    char c = Serial.read();       // get the character
    if (c == 'A' || c == 'C')
    {
      rx_byte = c;
      Serial.println(rx_byte);
    }
    
  }
      if (rx_byte == 'A')
        Serial.println("programming as an Admin card");
      else if (rx_byte == 'C')
        Serial.println("programming as an Commuter card");
      else
        Serial.println("this'll be a useless card");
    
  Serial.println("please scan an NTAG 215 nfc card");
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeout);
    if (success) 
      { 
        // Display some basic information about the card
        Serial.print("  UID Length: "); 
        Serial.print(uidLength, DEC); 
        Serial.println(" bytes");
        Serial.print("  UID Value: ");
        nfc.PrintHex(uid, uidLength);
            
    if (uidLength == 7)
    {
      uint8_t data[32];
      
      // We probably have an NTAG2xx card (though it could be Ultralight as well)
      Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");    
      
      // NTAG2x3 cards have 39*4 bytes of user pages (156 user bytes),
      // starting at page 4 ... larger cards just add pages to the end of
      // this range:
      
      // See: http://www.nxp.com/documents/short_data_sheet/NTAG203_SDS.pdf

      // TAG Type       PAGES   USER START    USER STOP
      // --------       -----   ----------    ---------
      // NTAG 203       42      4             39
      // NTAG 213       45      4             39
      // NTAG 215       135     4             129
      // NTAG 216       231     4             225      

      Serial.println("");
      Serial.println("Writing randomly generated codes to pages 4..129");
      Serial.println("");
      for (uint8_t i = 4; i < 129; i++) 
      {
        String randomKey = keyDB.GeneratePsuedoRandomKey();
        SetCardRecord(i, randomKey);
        Serial.print("Page: ");
        Serial.print(i);
        Serial.println(": still working");
      }
       if (rx_byte == 'A')
       {
        Serial.println("setting Admin code");  
        String key = keyDB.validAdminKeys[0];
        SetCardRecord(keyDB.initializationPageOne, key.substring(0,4));
        SetCardRecord(keyDB.initializationPageTwo, key.substring(4,8));
       }  
       if (rx_byte == 'C')
       {
        Serial.println("setting Commuter code");  
        String key = keyDB.validOneTimeKeys[0];
        Serial.println(key);
        SetCardRecord(keyDB.initializationPageOne, key.substring(0,4));
        SetCardRecord(keyDB.initializationPageTwo, key.substring(4,8));
       }  
    }
    else
    {
      Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    }
    
    // Wait a bit before trying again
    Serial.println("save successful");
}
delay(3000);
Serial.flush(); 
while(Serial.available()){Serial.read();}
}

void SetCardRecord(uint8_t page, String key)
{
    char buf[31];
    key.toCharArray(buf,30);
    nfc.ntag2xx_WritePage(page, buf);  
}
