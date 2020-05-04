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
String cardRecord;                       // defines a variable to check the success of an NFC record scan
uint8_t firstAdminPage = 6;
uint8_t secondAdminPage = 7;
uint8_t firstCommuterPage = 4;
uint8_t secondCommuterPage = 5;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID after NFC card scan
uint8_t record[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned record after NFC card scan
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type) for NFC card scan
uint16_t timeout = 5000;                  // defines a variable to timeout the card reader function, in ms
String cardCode;                          // defines a string variable to check against known card codes
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
  uint8_t pageOne = EEPROM.read(20);
  uint8_t pageTwo = EEPROM.read(21);
  uint8_t firstTime = EEPROM.read(22);
  GetStoredCode();
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED off
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeout);
  //success = nfc.ntag2xx_ReadPage(4, record);

  if (success) 
  { 
    // Display some basic information about the card
    Serial.print("  UID Length: "); 
    Serial.print(uidLength, DEC); 
    Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    //nfc.PrintHexChar(record, 4);
    //goodKey.toCharArray(record, 8);
    
    String initializationRecord = GetCardRecord(keyDB.initializationPageOne, keyDB.initializationPageTwo);
    bool admin;
    admin = keyDB.Admin(initializationRecord);
    
    bool first;
    first = keyDB.Initialization(initializationRecord);
    if(first && firstTime ==1)
    {
        String key = keyDB.replacementKey;
        SetCardRecord(keyDB.initializationPageOne, key.substring(0,4));
        SetCardRecord(keyDB.initializationPageTwo, key.substring(4,8));
    }

    String commuterRecord = GetCardRecord(pageOne, pageTwo);
    Serial.print("Commuter record: ");  
    Serial.println(commuterRecord);
    bool commuter;
    commuter = (commuterRecord == GetStoredCode());

    if(admin)
    {
      Serial.println("Card validated");  
        digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on
        Serial.print("Admin record: ");  
        Serial.println(initializationRecord);

        delay(1000);                      // adding a delay to prevent inadvertent rescans
    }
    else if(commuter)
    {
        Serial.println("Card validated");  
        digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on
        Serial.print("commuter record: ");  
        Serial.println(commuterRecord);
        uint8_t newPageOne = random(4,129);
        uint8_t newPageTwo = random(4,129);
        Serial.print("newPageOne: ");
        Serial.println(newPageOne);
        Serial.print("newPageTwo: ");
        Serial.println(newPageTwo);
        EEPROM.put(20, newPageOne);
        EEPROM.put(21, newPageTwo);
        SetStoredCode(GetCardRecord(newPageOne,newPageTwo));
        delay(1000);                      // adding a delay to prevent inadvertent rescans
    }
    else if(first && firstTime !=1)
    {
        Serial.println("Card validated");  
        digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on
        Serial.print("Initialization record: ");  
        Serial.println(initializationRecord);
        uint8_t newPageOne = random(4,129);
        uint8_t newPageTwo = random(4,129);
        Serial.print("newPageOne: ");
        Serial.println(newPageOne);
        Serial.print("newPagTwo: ");
        Serial.println(newPageTwo);
        EEPROM.put(20, newPageOne);
        EEPROM.put(21, newPageTwo);
        SetStoredCode(GetCardRecord(newPageOne,newPageTwo));
        EEPROM.put(22, 1);
        String key = keyDB.replacementKey;
        SetCardRecord(keyDB.initializationPageOne, key.substring(0,4));
        SetCardRecord(keyDB.initializationPageTwo, key.substring(4,8));
        
        delay(1000);                      // adding a delay to prevent inadvertent rescans
    }    


    else
    {
      Serial.println("The card is invalid, no action taken");
      delay(1000);                      // adding a delay to prevent inadvertent rescans
    }

  delay(1000);
}
}

void Test(String key)
{
  Serial.print(key);
  if (keyDB.Admin(key))
  {
    Serial.println(" key in database");
  }
  else
  {
    Serial.println(" key is not in database");
  }
}

String GetCardRecord(uint8_t firstPage, uint8_t secondPage)
{
    String returnValue;
    String pageValue;

      nfc.ntag2xx_ReadPage(firstPage, record);   
        for(byte i=0; i < 4; i++)
        {
          //returnValue.concat(String(record[i] < 0x10 ? " 0" : " "));
          pageValue.concat(String((char)record[i]));
        }

      nfc.ntag2xx_ReadPage(secondPage, record);  
        for(byte i=0; i < 4; i++)
        {
          //returnValue.concat(String(record[i] < 0x10 ? " 0" : " "));
          pageValue.concat(String((char)record[i]));
        }
     returnValue.concat(pageValue);

    return returnValue.substring(0);
}

void SetCardRecord(uint8_t page, String key)
{
    char buf[31];
    key.toCharArray(buf,30);
    nfc.ntag2xx_WritePage(4, buf);  
}

void SetStoredCode(String key)
{
  byte value;
  Serial.println(key);
  //Serial.println(randoKey, HEX);
    for(byte i=0; i<8; i++)
    {
      byte k = i+10;
      EEPROM.put(k, key[i]);
      value = EEPROM.read(k);
      Serial.print(k);
      Serial.print("\t");
      Serial.print(value, HEX);
      Serial.println();
    }         
  Serial.println("code set in EEPROM");
}

String GetStoredCode()
{
    String returnValue;
    for(byte k=10; k<18; k++)
    {
      returnValue.concat(String((char)EEPROM.read(k)));
    }
    Serial.println(returnValue);    
    Serial.println("code recovered from EEPROM");
    return returnValue.substring(0);
}
