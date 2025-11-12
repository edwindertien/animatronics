/*****************************************************************************
   Dynamixel Reader

   All kinds of functions and variables having to do with reading Dynamixel
   Messages.

   Call the function DynamixelPoll() regularly.
   This will call
     void ProcessDynamixelMessage(int id, unsigned char length, unsigned char *Data)
     which you will have to provide yourself, whenever a dynamixel message was
     received.

   As a user, you are responsible for initializing the Serial port (Serial.begin())
 **********************************************************************/

#include "RS485Reader.h"

typedef enum
{
  WaitingForFirstHeaderByte,
  WaitingForSecondHeaderByte,
  WaitingForIDByte,
  WaitingForDataLengthByte,
  WaitingForRestOfMessage
}
d_receive_state;

//#define DEBUG (1) /* Do undef for no debugging*/

// Prototype for a function that the user will provide somewhere.
extern void ProcessRS485Data(int ID, int dataLength, unsigned char *Data);

void RS485Init(long bd, int RS485sr)
{
  Serial1.begin(bd);
  Serial1.setTX(0);
  Serial1.setRX(1);
  pinMode(RS485_SR, OUTPUT);
}

void RS485Poll()
{
  static d_receive_state ReceiveState;
  static unsigned char c;
  static char NumberOfDataBytesReceived = 0;
  static unsigned char addressBuffer = 0;
  static unsigned char Data[RS485_BUFFER_SIZE];
  static unsigned char DataLengthBuffer;
  static unsigned int checksumBuffer = 0;

  if (Serial1.available() > 0) {
    c = Serial1.read();
    switch (ReceiveState) {
      case WaitingForFirstHeaderByte:
        if (c == 0xFF) {// first byte of header
          ReceiveState = WaitingForSecondHeaderByte;
        }
        break;
      case WaitingForSecondHeaderByte:
        if (c == 0xFF) {
          ReceiveState = WaitingForIDByte;
        }
        else ReceiveState = WaitingForFirstHeaderByte;
        break;
      case WaitingForIDByte:
        addressBuffer = c; // store byte
        //      if(addressBuffer != BOARD_ID) {ReceiveState=WaitingForFirstHeaderByte; break;}
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
        if (NumberOfDataBytesReceived < RS485_BUFFER_SIZE)
        {
          Data[NumberOfDataBytesReceived] = c;
        } // // otherwise, we would have buffer overflow

        checksumBuffer += c;
        NumberOfDataBytesReceived++;

        if (NumberOfDataBytesReceived >= DataLengthBuffer) // actually, larger can never happen, but just in case...
        {
          //Serial.println(checksumBuffer);
          // Check if the data is correct by means of checksum.
          // Also, if we had a buffer overflow, we know for sure that the
          // data is invalid (because we only stored part of it, so in that case we
          // should not process it either. It would have been a faulty command anyway).
          if (((checksumBuffer & 0xFF) == 0xFF) && NumberOfDataBytesReceived < RS485_BUFFER_SIZE)
          {
            ProcessRS485Data(addressBuffer, DataLengthBuffer, Data);
          }
          ReceiveState = WaitingForFirstHeaderByte;
        }
        break;
    } // of switch
  }
}


void RS485Write(int id, int address, int value) {
  byte vel_hi = (byte)(value >> 8);
  byte vel_lo = (byte)(value & 0xFF);
  // calculate checksum
  byte checksum = ~(id + 0x05 + 0x03 + address + vel_lo + vel_hi);
  unsigned char buffer[] = {
    0xFF, 0xFF, // Header
    (byte) id,
    0x05, // Data length
    0x03, // Write command
    (byte) address, // Starting address (Dynamixel ram, goal velocity)
    vel_lo,
    vel_hi,
    checksum
  };

  RS485WriteBuf(buffer, 9);
}
void RS485WriteByte(int id, int address, int value) {
  // calculate checksum
  unsigned char checksum = ~(id + 0x04 + 0x03 + address + value);
  unsigned char buffer[] = {
    (byte)0xFF, (byte)0xFF, // Header
    (byte)id,
    (byte)0x04, // Data length
    (byte)0x03, // Write command
    (byte)address, // Starting address (Dynamixel ram, goal velocity)
    (byte)value,
    (byte)checksum
  };
  RS485WriteBuf(buffer, 8);
}


void RS485Read(int id, int address) {

  // calculate checksum
  byte checksum = ~(id + 0x04 + 0x02 + address + 0x02 );
  unsigned char buffer[] = {
    0xFF, 0xFF, // Header
    (byte)id,
    0x04, // Data length
    0x02, // Read command
    (byte)address, // Starting address (Dynamixel ram, goal velocity)
    0x02, // data length
    checksum
  };

  digitalWrite(RS485_SR, HIGH);
  Serial1.write(buffer, 8);
  Serial1.flush();
  digitalWrite(RS485_SR, LOW);
}

void RS485WriteBuf(unsigned char *buffer, int length) {

  digitalWrite(RS485_SR, HIGH);
  Serial1.write((uint8_t*)buffer, length);
  Serial1.flush(); // waits for the buffer to be empty
  digitalWrite(RS485_SR, LOW);
}

void RS485WriteBuffer(int id, unsigned char *buffer, int messagelength) {
  unsigned char length = messagelength + 2;
  unsigned char checksum;
  unsigned char transmitBuffer[20];
  for (int i = 0; i < messagelength; i++) {
    checksum += buffer[i];
  }
  transmitBuffer[0] = 0xFF;
  transmitBuffer[1] = 0xFF;
  transmitBuffer[2] = id;
  transmitBuffer[3] = length;
  transmitBuffer[4] = 0x03;
  for (int i = 5; i < messagelength + 5; i++) {
    transmitBuffer[i] = buffer[i - 5];
  }
  transmitBuffer[messagelength + 5] = ~(checksum + id + 0x03 + length);
  digitalWrite(RS485_SR, HIGH);
  Serial1.write((uint8_t*)transmitBuffer, length + 4);
  Serial1.flush(); // waits for the buffer to be empty
  digitalWrite(RS485_SR, LOW);
}
/*
  void RobotWrite(int board) {
  unsigned char length = 19; // 17 + 2
  int checksumBuffer = 0;
  for (int n = 0; n < 17; n++) {
    checksumBuffer += transmitBuffer[n];
  }
  unsigned char checksum = ~( board + length + 0x03 + checksumBuffer);
  unsigned char buff[length + 4] = {
    0xFF, 0xFF, // Header
    (unsigned char) board, //ID
    length, // length
    0x03, // write
    transmitBuffer[0], // x
    transmitBuffer[1], // y
    transmitBuffer[2], // mode
    transmitBuffer[3], // arm
    transmitBuffer[4],
    transmitBuffer[5],
    transmitBuffer[6],
    transmitBuffer[7],
    transmitBuffer[8],
    transmitBuffer[9],
    transmitBuffer[10], // arm
    transmitBuffer[11], // main gain
    transmitBuffer[12], // neck tilt
    transmitBuffer[13], // neck pan
    transmitBuffer[14], // valence
    transmitBuffer[15], // arousal (also blink)
    transmitBuffer[16], // belly
    checksum
  };
  RFWriteRaw(buff, length + 4);
  }*/



      
//   DynamixelWrite(dynamixelIDs[0], 28, map(channels[2],0,255,0,8));
//   DynamixelWrite(dynamixelIDs[1], 28, map(channels[2],0,255,0,8));
//   DynamixelWrite(dynamixelIDs[2], 28, map(channels[2],0,255,0,8));

// DynamixelWrite(dynamixelIDs[0], 30, map(channels[0], 0, 255, dynamixelMinPos[0]-dynamixelOffset[0], dynamixelMaxPos[0]-dynamixelOffset[0]));
// DynamixelWrite(dynamixelIDs[1], 30, map(channels[1], 0, 255, dynamixelMinPos[1]-dynamixelOffset[1], dynamixelMaxPos[1]-dynamixelOffset[1]));

// DynamixelWrite(dynamixelIDs[2], 30, map(channels[1], 0, 255, dynamixelMinPos[2]-dynamixelOffset[2], dynamixelMaxPos[2]-dynamixelOffset[2]));

// for (int n = 0; n < 2; n++) {
//   DynamixelWrite(dynamixelIDs[n], 34, map(channels[15],0,255,0,1023)); // 28 -> gain


// }
// for(int i = 2; i<7; i++){
// DynamixelWrite(dynamixelIDs[i], 28, map(channels[15],0,255,0,8));
// }


// DynamixelWrite(dynamixelIDs[0], 30, map(channels[4]-(127-channels[3]), 127, 255, dynamixelMinPos[0]-dynamixelOffset[0], dynamixelMaxPos[0]-dynamixelOffset[0]));
// DynamixelWrite(dynamixelIDs[1], 30, map(channels[4]+(127-channels[3]), 127, 255, dynamixelMinPos[1]-dynamixelOffset[1], dynamixelMaxPos[1]-dynamixelOffset[1]));

// DynamixelWrite(dynamixelIDs[2], 30, map(channels[5]-(127-channels[6]), 0, 255, dynamixelMinPos[2]-dynamixelOffset[2], dynamixelMaxPos[2]-dynamixelOffset[2]));
// DynamixelWrite(dynamixelIDs[3], 30, map(channels[5]+(127-channels[6]), 0, 255, dynamixelMinPos[3]-dynamixelOffset[3], dynamixelMaxPos[3]-dynamixelOffset[3]));

// DynamixelWrite(dynamixelIDs[4], 30, map(channels[0], 0, 255, dynamixelMinPos[4]-dynamixelOffset[4], dynamixelMaxPos[4]-dynamixelOffset[4]));

// DynamixelWrite(dynamixelIDs[5], 30, map(channels[1], 0, 255, dynamixelMinPos[5]-dynamixelOffset[5], dynamixelMaxPos[5]-dynamixelOffset[5]));
// DynamixelWrite(dynamixelIDs[6], 30, map(channels[1], 0, 255, dynamixelMinPos[6]-dynamixelOffset[6], dynamixelMaxPos[6]-dynamixelOffset[6]));

// pinMode(22,OUTPUT);
// pinMode(21,OUTPUT);

// // dynamixel
//   Serial1.begin(1000000);
//   Serial1.setTX(0);
//   Serial1.setRX(1);
//   pinMode(RS485_SR, OUTPUT);

//    for (int n = 0; n < 2; n++) {
//     DynamixelWrite(dynamixelIDs[n], 28, 0); // 28 -> gain
//     DynamixelWrite(dynamixelIDs[n], 29, 0); // 28 -> gain
//     DynamixelWrite(dynamixelIDs[n], 16, 0); // (1 for data, 0 for silence) 
//   }

//    for (int n = 2; n < 7; n++) {
//     DynamixelWrite(dynamixelIDs[n], 28, 0); // 28 -> gain
//     DynamixelWrite(dynamixelIDs[n], 16, 0); // (1 for data, 0 for silence)
//   }