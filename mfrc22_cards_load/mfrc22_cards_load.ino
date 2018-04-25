/**
* ----------------------------------------------------------------------------
* This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
* for further details and other examples.
*
* NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
*
* Released into the public domain.
* ----------------------------------------------------------------------------
* This sample shows how to read and write data blocks on a MIFARE Classic PICC
* (= card/tag).
*
* BEWARE: Data will be written to the PICC, in sector #1 (blocks #4 to #7).
*
*
* Typical pin layout used:
* -----------------------------------------------------------------------------------------
*             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
*             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
* Signal      Pin          Pin           Pin       Pin        Pin              Pin
* -----------------------------------------------------------------------------------------
* RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
* SPI SS      SDA(SS)      10            53        D10        10               10
* SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
* SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
* SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*
*/

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         5           // Configurable, see typical pin layout above
#define SS_PIN          53          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

unsigned int piccSize = 0;
byte nuidPICC[4];
unsigned int numb = 0;
byte flag;

/**
* Initialize.
*/
void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(9600); // Initialize serial communications with the PC
	while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();        // Init SPI bus
	mfrc522.PCD_Init(); // Init MFRC522 card

	// Prepare the key (used both as key A and as key B)
	// using FFFFFFFFFFFFh which is the default at chip delivery from the factory
	for (byte i = 0; i < 6; i++) {
		key.keyByte[i] = 0xFF;
	}
	flag = 0;
	Serial.println("Step 1: Lay card on rfid.");
	Serial.println("Step 2: Digit room number on serial.");
	Serial.println("Step 3: Lay master and wait saving.");
	Serial.println("Step 4: Repeat for each card.");
	Serial.println();
}
void loop() {
	Serial.println("Waiting Card..");
	Serial.println("Insert room number: ");
	while (Serial) {
		CheckCard();
		if (Serial.available() > 0) {
			numb = Serial.parseInt();
			Serial.println(numb);
			Serial.println("Waiting Master for save...");
			while (Serial && flag == 0)
				SaveDataToMaster(numb);
		}
		flag = 0;
	}
}

void blink() {
	digitalWrite(LED_BUILTIN, HIGH);
	delay(500);
	digitalWrite(LED_BUILTIN, LOW);
	delay(500);
}

void CheckCard() {

	// Look for new cards
	if (!mfrc522.PICC_IsNewCardPresent())
		return;

	// Select one of the cards
	if (!mfrc522.PICC_ReadCardSerial())
		return;

	for (byte i = 0; i < 4; i++) {
		nuidPICC[i] = mfrc522.uid.uidByte[i];
	}

	Serial.print(F("Card UID:"));
	dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
	Serial.println();
	Serial.print(F("PICC type: "));
	MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
	Serial.println(mfrc522.PICC_GetTypeName(piccType));
	// Check for compatibility
	if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
		&&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
		&&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
		Serial.println(F("This sample only works with MIFARE Classic cards."));
		return;
	}

	// Halt PICC
	mfrc522.PICC_HaltA();
	// Stop encryption on PCD
	mfrc522.PCD_StopCrypto1();

}

void SaveDataToMaster(unsigned int n) {
	byte sector = 1;
	byte blockAddr = 8;
	unsigned int myRoom = n;
	byte high = highByte(myRoom);
	byte low = lowByte(myRoom);
	byte dataBlock[] = {
		nuidPICC[0],nuidPICC[1],nuidPICC[2],nuidPICC[3],
		high,low,0,0,
		0,0,0,0,
		0,0,0,0 };
	byte trailerBlock = 11;
	MFRC522::StatusCode status;
	byte buffer[18];
	byte size = sizeof(buffer);

	// Look for new cards
	if (!mfrc522.PICC_IsNewCardPresent())
		digitalWrite(LED_BUILTIN, HIGH);

	// Select one of the cards
	if (!mfrc522.PICC_ReadCardSerial())
		return;
	// Show some details of the PICC (that is: the tag/card)
	blink();
	Serial.print(F("Card UID:"));
	dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
	Serial.println();
	Serial.print(F("PICC type: "));
	MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
	Serial.println(mfrc522.PICC_GetTypeName(piccType));

	// Check for compatibility
	if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
		&&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
		&&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
		Serial.println(F("This sample only works with MIFARE Classic cards."));
		return;
	}
	//picc size for dumpinfo
	if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI)
		piccSize = 320;
	else if (piccType == MFRC522::PICC_TYPE_MIFARE_1K)
		piccSize = 1024;
	else if (piccType == MFRC522::PICC_TYPE_MIFARE_4K)
		piccSize = 4096;


	unsigned int sizeCard = piccSize / 16;

	// Authenticate using key A
	Serial.println(F("Authenticating....Trailer block: "));
	Serial.print(trailerBlock);
	Serial.println();
	status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
	if (status != MFRC522::STATUS_OK) {
		Serial.print(F("PCD_Authenticate() failed: "));
		Serial.println(mfrc522.GetStatusCodeName(status));
		return;
	}

	while (blockAddr < sizeCard - 1) {
		if (blockAddr == trailerBlock) {
			Serial.println("Trovato trailer block ");
			Serial.println(blockAddr);
			trailerBlock = trailerBlock + 4;
			blockAddr++;
			// Authenticate using key A
			Serial.println(F("Authenticating....Next Trailer block: "));
			Serial.print(trailerBlock);
			Serial.println();
			status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
			if (status != MFRC522::STATUS_OK) {
				Serial.print(F("PCD_Authenticate() failed: "));
				Serial.println(mfrc522.GetStatusCodeName(status));
				return;
			}
		}
		Serial.print("Sono nel blocco: ");
		Serial.print(blockAddr);
		Serial.println();
		// Read data from block
		status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
		if (status != MFRC522::STATUS_OK) {
			Serial.print(F("MIFARE_Read() failed: "));
			Serial.println(mfrc522.GetStatusCodeName(status));
			break;
		}
		if (buffer[0] == 0 &&
			buffer[1] == 0 &&
			buffer[2] == 0 &&
			buffer[3] == 0 &&
			buffer[4] == 0 &&
			buffer[5] == 0) {
			// Write data to the block
			Serial.print(F("Writing data into block "));
			Serial.print(blockAddr);
			Serial.println(F(" ..."));
			Serial.print("Room Number: ");
			Serial.print(myRoom);
			Serial.println();
			dump_byte_array(dataBlock, 16);
			status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
			if (status != MFRC522::STATUS_OK) {
				Serial.print(F("MIFARE_Write() failed: "));
				Serial.println(mfrc522.GetStatusCodeName(status));
			}
			blockAddr++;
			FreeBuffer(buffer);
			break;
		}
		Serial.println("blocco pieno");
		blockAddr++;
		FreeBuffer(buffer);
	}
	// Dump the sector data
//	Serial.println(F("Current data in sector:"));
//	mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
	// Halt PICC
	mfrc522.PICC_HaltA();
	// Stop encryption on PCD
	mfrc522.PCD_StopCrypto1();
	FreeArray();
	Serial.println();
	Serial.println("Load complete...Please remove card");
	flag = 1;
}

void FreeBuffer(byte *buff) {
	for (int i = 0; i < 18; i++)
		buff[i] = 0;
}
void FreeArray() {
	for (int i = 0; i < 4; i++) {
		nuidPICC[i] = 0;
	}
}

unsigned int ByteToInt(unsigned char highB, unsigned char lowB) {
	int value;
	value = highB; //Assegna il valore del byte piu' alto
	value = value << 8; //Shifta di 8 posizioni essendo 16 bit=2byte es. 00000000 00000011->11000000 00000000
	value |= lowB; // or logica tra il valore shiftato e il bytelow per il valore finale
	return value;
}

void dump_byte_array(byte *buffer, byte bufferSize) {
	for (byte i = 0; i < bufferSize; i++) {
		Serial.print(buffer[i] < 0x10 ? " 0" : " ");
		Serial.print(buffer[i], HEX);
	}

}