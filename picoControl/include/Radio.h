#ifndef RADIO_H
#define RADIO_H

#include <Arduino.h>

#define RF_SET 3
#define RF_BUFFER_SIZE 64
#define BOARD_ID 13


void RadioPoll(void);
void nudgeTimeOut(void);
int getTimeOut(void);

void RFgetSettings(void) ;
void RFsetSettings(int channel);
void RFinit(void);


#endif