#include "Arduino.h"
#include <SoftwareSerial.h>

#ifndef MoProtocol_h
#define MoProtocol_h

class MoProtocol
{
  public:
    MoProtocol(byte address);
    void start(int, uint8_t pin_rx, uint8_t pin_tx,uint8_t direction_tx);    
    bool haveData();    
    bool parseTelegram(byte *data, byte *res);
    bool getTelegram(byte *telegram);
    bool getSerialTelegram(byte *telegram);
    void buildTelegram(byte to, byte command, byte data, byte *res);
    void debugTelegram(byte *data, String message);
    void sendTelegram(byte *data);
  private:
    char _address;
    uint8_t _pin_direction;
    SoftwareSerial *channel;
    bool _isValid(byte *data);
    bool _isForMe(byte *data);
};

#endif

