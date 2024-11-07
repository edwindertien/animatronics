/*****************************************************************************
 * Dynamixel Reader
 * 
 * All kinds of functions and variables having to do with reading Dynamixel
 * Messages.   
 * 
 * Call the function DynamixelPoll() regularly.
 * This will call 
 *   void ProcessDynamixelMessage(int id, unsigned char length, unsigned char *Data)
 *   which you will have to provide yourself, whenever a dynamixel message was
 *   received.   
 *   
 * As a user, you are responsible for initializing the Serial port (Serial.begin())    
 **********************************************************************/


#include "Arduino.h"
#include "DynamixelReader.h"


//#define DEBUG (1) /* Do undef for no debugging*/

// Prototype for a function that the user will provide somewhere.
void ProcessDynamixelData(const unsigned char ID, const int dataLength, const unsigned char* const Data);

volatile int timeOutValue;


typedef enum
{
  WaitingForFirstHeaderByte,
  WaitingForSecondHeaderByte,
  WaitingForIDByte,
  WaitingForDataLengthByte,
  WaitingForRestOfMessage
} 
e_receive_state;


void DynamixelPoll(void)  
{ 
  static e_receive_state ReceiveState;
  static unsigned char c;
  static char NumberOfDataBytesReceived = 0;
  static unsigned char addressBuffer = 0;
  static unsigned char Data[DYNAMIXEL_BUFFER_SIZE];
  static unsigned char DataLengthBuffer;
  static unsigned int checksumBuffer = 0;

  if (Serial1.available()>0)
  {
    c = Serial1.read();
//Serial.print(c);
    switch (ReceiveState)
    {
    case WaitingForFirstHeaderByte:
      if (c == 0xFF) // first byte of header
      {

        ReceiveState = WaitingForSecondHeaderByte;
      }
      break;
    case WaitingForSecondHeaderByte:
      if (c == 0xFF) 
      {
        //          toggle(LED_YELLOW);
        ReceiveState = WaitingForIDByte;
      } 
      else ReceiveState = WaitingForFirstHeaderByte;
      break;
    case WaitingForIDByte:
      // store byte
      addressBuffer = c;
      //Serial.println(addressBuffer);
      if(addressBuffer != BOARD_ID) {ReceiveState=WaitingForFirstHeaderByte; break;}
      checksumBuffer = c;
      ReceiveState = WaitingForDataLengthByte;
      break;
    case WaitingForDataLengthByte:      
      DataLengthBuffer = c;
      NumberOfDataBytesReceived = 0;
      checksumBuffer += c;
      ReceiveState = WaitingForRestOfMessage;
      break;
    case WaitingForRestOfMessage:
      if (NumberOfDataBytesReceived < DYNAMIXEL_BUFFER_SIZE)
      {
        Data[NumberOfDataBytesReceived] = c;
      } // // otherwise, we would have buffer overflow

      checksumBuffer += c;
      NumberOfDataBytesReceived++;

      if (NumberOfDataBytesReceived >= DataLengthBuffer) // actually, larger can never happen, but just in case...
      {
        // Check if the data is correct by means of checksum.
        // Also, if we had a buffer overflow, we know for sure that the
        // data is invalid (because we only stored part of it, so in that case we
        // should not process it either. It would have been a faulty command anyway).
        if(((checksumBuffer & 0xFF) == 0xFF) && NumberOfDataBytesReceived<DYNAMIXEL_BUFFER_SIZE)
        {
          timeOutValue=0;
          ProcessDynamixelData(addressBuffer, DataLengthBuffer, Data); 
        } 
        ReceiveState = WaitingForFirstHeaderByte;
      }
      break;
    } // of switch
  }
}

int getTimeOut(void){
  return(timeOutValue);
}
void nudgeTimeOut(void){
  timeOutValue++;
}
