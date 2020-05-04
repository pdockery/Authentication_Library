#include "Arduino.h"
#include <String.h>

class KeyDatabase
{
	// My understanding of cryptography is likely not sufficient for production. Here is my understanding:
	// a 32 bit key can be expressed as a hexadecimal string 8 characters long
	// at 32 bits the number of possible keys is 2^32
	// In a brute force attack if we allow 1 key attempt per second the maximum amount of time required to find a single key is about 136 years (2^32/(60*60*24*365.25)).
	// The average would take about half that time (68 years)
	// For every key we add the average brute force time then becomes 68 years/n.
	// We then can have 68 random keys and expect that it will take about a year for any one of them to be guessed in a brute force.
	// This should be sufficient for our purposes as this is a temporary solution and the means of attack for this product
	// would likely be social engineering or theft to get a valid code or opening the box to splice wiring.
//	private:
//		int numAdminKeys = 2;
//		String validAdminKeys[2] = {
//			"5338B995", // Company A
//			"B9873D79", // Company B
//		};
//    int numOneTimeKeys = 2;
//    String validOneTimeKeys[2] = {
//      "2C6C6C01", // Company A
//      "1DDDB85A", // Company B
//    };
//    uint8_t initializationPageOne = 4;
//    uint8_t initializationPageTwo = 5;  

	
	public:
		KeyDatabase();
		~KeyDatabase();
		bool Admin(String key);
    bool Initialization(String key);
		//int GetNumberOfKeys();
		String GeneratePsuedoRandomKey();
      int numAdminKeys = 2;
    String validAdminKeys[2] = {
      "5338B995", // Company A
      "B9873D79", // Company B
    };
    int numOneTimeKeys = 2;
    String validOneTimeKeys[2] = {
      "2C6C6C01", // Company A
      "1DDDB85A", // Company B
    };
    String replacementKey = "0CDA20EB";
    uint8_t initializationPageOne = 4;
    uint8_t initializationPageTwo = 5;  
};
