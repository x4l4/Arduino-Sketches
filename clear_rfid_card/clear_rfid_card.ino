/**
  ----------------------------------------------------------------------------
  This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
  for further details and other examples.

  NOTE: The library file MFRC522.h has a lot of useful info. Please read it.

  Released into the public domain.
  ----------------------------------------------------------------------------
  This sample shows how to read and write data blocks on a MIFARE Classic PICC
  (= card/tag).

  BEWARE: Data will be written to the PICC, in sector #1 (blocks #4 to #7).


  Typical pin layout used:
  -----------------------------------------------------------------------------------------
              MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
              Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
  Signal      Pin          Pin           Pin       Pin        Pin              Pin
  -----------------------------------------------------------------------------------------
  RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
  SPI SS      SDA(SS)      10            53        D10        10               10
  SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
  SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
  SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

*/

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         5           // Configurable, see typical pin layout above
#define SS_PIN          53          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

unsigned int piccSize = 0;
byte nuidPICC[4];

/**
  Initialize.
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
  Serial.println("Waiting Card..");
  Serial.println();
}
void loop() {
  //CheckCard();
  SaveDataToMaster();
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

  SaveDataToMaster();

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

}

void SaveDataToMaster() {
  byte sector = 1;
  byte blockAddr = 8;
  byte dataBlock[] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
  };
  byte trailerBlock = 11;
  MFRC522::StatusCode status;

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
    digitalWrite(LED_BUILTIN, HIGH);

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
    return;

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
    dump_byte_array(dataBlock, 16);
     Serial.println();
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    blockAddr++;
  }
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }

}
