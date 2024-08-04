#include "Radio.h"
#include "Arduino.h"

void ProcessRadioData(int ID, int dataLength, unsigned char *Data);
volatile unsigned long timeOutValue;


typedef enum
{
  WaitingForFirstHeaderByte,
  WaitingForSecondHeaderByte,
  WaitingForIDByte,
  WaitingForDataLengthByte,
  WaitingForRestOfMessage
}
e_receive_state;


void RadioPoll()
{
  static e_receive_state ReceiveState;
  static unsigned char c;
  static char NumberOfDataBytesReceived = 0;
  static unsigned char addressBuffer = 0;
  static unsigned char Data[RF_BUFFER_SIZE];
  static unsigned char DataLengthBuffer;
  static unsigned int checksumBuffer = 0;

  if (Serial2.available() > 0)
  {
    c = Serial2.read();

#ifdef DEBUG
    Serial.print(c, HEX);
    Serial.print(" ");
#endif
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
        if (addressBuffer != BOARD_ID) {
          ReceiveState = WaitingForFirstHeaderByte;
          break;
        }
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
        if (NumberOfDataBytesReceived < RF_BUFFER_SIZE)
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
          if (((checksumBuffer & 0xFF) == 0xFF) && NumberOfDataBytesReceived < RF_BUFFER_SIZE)
          {
            timeOutValue = 0;
            ProcessRadioData(addressBuffer, DataLengthBuffer, Data);
#ifdef DEBUG
            Serial.print("P");
#endif
          }
          else
          {
#ifdef DEBUG
            Serial.print("I");
#endif
          }
          ReceiveState = WaitingForFirstHeaderByte;
        }
        break;
    } // of switch
  }
}

int getTimeOut(void) {
  return (timeOutValue);
}
void nudgeTimeOut(void) {
  if(timeOutValue<255)timeOutValue++;
}




void RFsetSettings(int channel) {
  digitalWrite(RF_SET, LOW);  // pulling SET to low will put apc220 in config mode
  delay(200);                 // stabilize please
  switch (channel) {
    case 1:
      Serial2.println("WR 424000 3 9 3 0");
      break;
    case 2:
      Serial2.println("WR 434000 3 9 3 0");
      break;
    case 3:
      Serial2.println("WR 444000 3 9 3 0");
      break;
    case 4:
      Serial2.println("WR 430000 3 9 3 0");
      break;
    case 5:
      Serial2.println("WR 440000 3 9 3 0");
      break;
    case 6:
      Serial2.println("WR 450000 3 9 3 0");
      break;
    default:
      break;
  }
  // format: WR Frequency RFDataRate OutputPower UART-Rate Series check
  // Frequency: Unit is KHz,for example 434MHz is 434000
  // RF Data Rate: 1,2,3 and 4 refer to 2400,4800,9600,19200bps
  // Output Power: 0 to 9, 9 means 13dBm(20mW)
  // UART Rate: 0,1,2,3,4,5 and 6 refers to 1200,2400,4800,9600, 19200,38400,57600bps
  // Series Checkout: Series checkout:0 means no check,1 means even parity,2 means odd parity.

  delay(50);
#ifdef DEBUG
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
#endif
  digitalWrite(RF_SET, HIGH);  // put apc220 back in operation
  delay(200);
}
void RFinit(void){
  Serial2.begin(9600);
  Serial2.setTX(8);
  Serial2.setRX(9);
  pinMode(RF_SET,OUTPUT);
  digitalWrite(RF_SET,HIGH);
}

void RFgetSettings(void) {
  digitalWrite(RF_SET, LOW);  // pulling SET to low will put apc220 in config mode
  delay(10);                   // stabilize please
  Serial2.println("RD");       // ask for data
  delay(10);
#ifdef DEBUG
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
#endif
  digitalWrite(RF_SET,HIGH);  // put apc220 back in operation
  delay(200);
}


