#ifndef __DYNAMIXEL_READER_H__
#define __DYNAMIXEL_READER_H__

#define DYNAMIXEL_BUFFER_SIZE (256) /* Just a lot so that it will not overflow */
#define BOARD_ID 10

#define toggle(pin) digitalWrite(pin, !digitalRead(pin)) 

// Prototype for the function we need to call from outside
void DynamixelPoll(void);
void nudgeTimeOut(void);
int getTimeOut(void);

#endif
