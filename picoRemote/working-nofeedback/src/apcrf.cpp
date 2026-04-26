#include "Arduino.h"
#include "apcrf.h"

apcRF::apcRF(int serialPin) {
  _serialPin = serialPin;
  // Initialize serial communication on Serial2 (you can change this to another serial port if needed)
  Serial2.setRX(9);  // APC220 433MHz radio on Serial2
  Serial2.setTX(8);
  Serial2.begin(9600);  // Adjust baud rate if necessary
  pinMode(_serialPin,OUTPUT);
}

void apcRF::RobotWrite(int board, unsigned char *message, int messagelength) {
  unsigned char length = messagelength + 2;
  int checksumbuffer = board + length + 0x03;
  
  for (int i = 0; i < messagelength; i++) {
    checksumbuffer += message[i];
  }
  
  unsigned char checksum = ~(checksumbuffer);
  unsigned char buff[length + 4];
  
  buff[0] = 0xFF;
  buff[1] = 0xFF;
  buff[2] = (unsigned char) board;
  buff[3] = length;
  buff[4] = 0x03;
  
  for (int i = 5; i < length + 3; i++) {
    buff[i] = message[i - 5];
  }
  
  buff[length + 3] = checksum;

  RFWriteRaw(buff, length + 4);
}

/********************************************************************
   RAW SERIAL COMMUNICATION USING RS485 PROTOCOL

   RS485 uses an extra bit, being the Send/Receive bit. Therefore, we need
   'wrappers' around the Serial.XXX()'s that takes care of handling it.
   These functions are below.
 ********************************************************************/

// Writes the characters in buffer to the RS485. Blocks until the
// sending has finished.

void apcRF::RFWriteRaw(unsigned char *buffer, int length) {
  // If you need to use an RS485 driver, you would toggle the direction pin here (RS485_SR)
  
  if (digitalRead(LED_BUILTIN) == 0) {
    digitalWrite(LED_BUILTIN, HIGH); 
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  digitalWrite(_serialPin,HIGH);
  // Write the buffer to Serial2 (RS485)
  Serial2.write((uint8_t*)buffer, length);
  Serial2.flush();  // Wait for the buffer to be emptied
  digitalWrite(_serialPin,LOW);
}