#include <EEPROM.h>
struct MyRoom{
  byte master[4]={147,255,62,2};
  unsigned int room=500;
};
MyRoom stanza;

void setup() {
  EEPROM.put(EEPROM.length() - 6, stanza);
}

void loop() {

}
