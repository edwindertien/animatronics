#ifndef APCRF_h
#define APCRF_h

#include <Arduino.h>

class apcRF {
  public:
    apcRF(int serialPin); // Constructor to initialize the RS485
    void RobotWrite(int board, unsigned char *message, int messagelength);
  
  private:
    int _serialPin; // RS485 serial pin
    void RFWriteRaw(unsigned char *buffer, int length);
};

#endif