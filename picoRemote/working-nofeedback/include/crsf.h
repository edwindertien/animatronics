// a definition of (most of ) the CRSF protocol can be found here: 
// https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md

#ifndef CRSF_H
#define CRSF_H

#include <Arduino.h>

#define RADIO_ADDRESS                  0xEA
#define ADDR_MODULE                    0xEE  //  Crossfire transmitter
#define TYPE_CHANNELS                  0x16
// internal crsf variables
#define CRSF_CHANNEL_MIN 172
#define CRSF_CHANNEL_MID 991
#define CRSF_CHANNEL_MAX 1811
#define CRSF_TIME_NEEDED_PER_FRAME_US   1100 // 700 ms + 400 ms for potential ad-hoc request
#define CRSF_TIME_BETWEEN_FRAMES_US     4000 // 4 ms 250Hz
#define CRSF_PAYLOAD_OFFSET offsetof(crsfFrameDef_t, type)
#define CRSF_MAX_CHANNEL 16
#define CRSF_FRAME_SIZE_MAX 64
#define SERIAL_BAUDRATE 420000
#define CRSF_MSP_RX_BUF_SIZE 128
#define CRSF_MSP_TX_BUF_SIZE 128
#define CRSF_PAYLOAD_SIZE_MAX   60
#define CRSF_PACKET_LENGTH 22
#define CRSF_PACKET_SIZE  26
#define CRSF_FRAME_LENGTH 24   // length of type + payload + crc

extern uint8_t crsfPacket[CRSF_PACKET_SIZE];
extern int rcChannels[CRSF_MAX_CHANNEL];
extern uint32_t crsfTime;

void crsfInit();
void crsfPreparePacket(uint8_t packet[], int channels[]);
void crsfSendPacket();
void crsfCallback();

enum chan_order{
    THROTTLE, 
    AILERON,
    ELEVATOR,
    RUDDER,
    AUX1,  // (CH5)  ARM switch for Expresslrs
    AUX2,  // (CH6)  angel / airmode change
    AUX3,  // (CH7)  flip after crash
    AUX4,  // (CH8) 
    AUX5,  // (CH9) 
    AUX6,  // (CH10) 
    AUX7,  // (CH11)
    AUX8,  // (CH12)
};

#endif