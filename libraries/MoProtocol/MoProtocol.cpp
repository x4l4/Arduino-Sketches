#include "Arduino.h"
#include "MoProtocol.h"
#include <SoftwareSerial.h>

#define RS485Transmit    HIGH
#define RS485Receive     LOW

MoProtocol::MoProtocol(byte address){
  _address = address;
}

void MoProtocol::start(int baud, uint8_t pin_rx, uint8_t pin_tx, uint8_t pin_direction) {
  channel = new SoftwareSerial(pin_rx, pin_tx); // RX, TX
  _pin_direction = pin_direction;
  pinMode(_pin_direction, OUTPUT); 
  digitalWrite(_pin_direction, RS485Receive);
  channel->begin(baud);
  Serial.println("waiting for commands...");
}

bool MoProtocol::haveData() {
  return Serial.available() || channel->available();
}

void MoProtocol::debugTelegram(byte *data, String message){
  String dest = "";
  for(int i = 0; i< 7; i++){
      dest += " 0x";
      dest += String(data[i], HEX);
  }
  Serial.println(dest + " "+message);
}

void MoProtocol::buildTelegram(byte to, byte command, byte data, byte *res){
  byte chksum = ((0x00 + _address + to + command + data + 0xFF) % 256);
  res[0] = 0x00;
  res[1] = _address;
  res[2] = to;
  res[3] = command;
  res[4] = data;
  res[5] = chksum;
  res[6] = 0xFF;
}

bool MoProtocol::getSerialTelegram(byte *telegram){
  bool found_start = 0;
  int byte_pos = 0;
  while(Serial.available()) {
    delay(5);
    char c1 = Serial.read();
    char c2 = Serial.read();    
    
    if(c1 == '\n' || c2 == '\n')
      continue;
    if(c1 == '\r' || c2 == '\r')
      continue;
    
    String tok = String("0x");
    tok.concat(c1);
    tok.concat(c2);
    byte byteReceived = strtoul(tok.c_str(), 0, 16);
    if(byteReceived != 0 && !found_start)
      continue;
      
    if(byteReceived == 0) {
      found_start = true;
      byte_pos = 0;
    }
    
      
    telegram[byte_pos] = byteReceived;
    byte_pos++;  
    if(byte_pos > 7) {
      Serial.print("bufer");
      return false;
    }
    if(byteReceived == 0xFF && byte_pos == 7) {
      debugTelegram(telegram, "telegramma ricevuto");
      return true;
    }      
  }
  Serial.println("reset");
  return false;
}

bool MoProtocol::getTelegram(byte *telegram){
  bool found_start = 0;
  int byte_pos = 0;
  while(haveData()) {
    byte byteReceived = channel->read();
    if(byteReceived != 0 && !found_start)
      continue;
      
    if(byteReceived == 0) {
      found_start = true;
      byte_pos = 0;
    }
      
    telegram[byte_pos] = byteReceived;
    byte_pos++;  
    if(byteReceived == 0xFF && byte_pos > 7) {
      debugTelegram(telegram, "telegramma ricevuto");
      return true;
    }      
  }
  return false;
}

void MoProtocol::sendTelegram(byte *telegram) {
  digitalWrite(_pin_direction, RS485Transmit);
  for(int i = 0; i< 7; i++){
    channel->print(telegram[i]);
  }
  digitalWrite(_pin_direction, RS485Receive);
}

bool MoProtocol::parseTelegram(byte *data, byte *res){
  res[0] = 0x00;
  res[1] = 0x00;
  
  if(!MoProtocol::_isValid(data)){
    Serial.println("FAILED");
    return false;
  }
  
  
  if(!MoProtocol::_isForMe(data)){
     Serial.println("IGNORED");
     return false;
  }
  
  res[0] = data[3];
  res[1] = data[4];
  
  
  return true;
}

bool MoProtocol::_isValid(byte *data){
  int sum = 0;
  for(int i = 0; i< 7; i++){
    if(i == 5){
      continue;
    }
    Serial.print(sum);
    sum += data[i];
    Serial.print(" + ");
    Serial.print(data[i]);
    Serial.print(" = ");
    Serial.println(sum);
    
  }
  
  Serial.print(sum%256);
  Serial.print(" == ");
  Serial.println(data[5]);
  return ((sum%256) == data[5]);
}

bool MoProtocol::_isForMe(byte *data){
  return (data[2] == _address);
}


