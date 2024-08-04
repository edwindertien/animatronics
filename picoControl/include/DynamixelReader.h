#ifndef __DYNAMIXEL_READER_H__
#define __DYNAMIXEL_READER_H__

#include <Arduino.h>

#define DYNAMIXEL_BUFFER_SIZE (128) /* Just a lot so that it will not overflow */
#define BOARD_ID 13
#define RS485_SR 2

#define limit(number, min, max) (((number) > (max)) ? (max) : (((number) < (min)) ? (min) : (number)))
// processing and checking:

#define toggle(pin) digitalWrite(pin, !digitalRead(pin)) 
#define	outb(addr, data)	addr = (data)
#define	inb(addr)		(addr)
#define BV(bit)			(1<<(bit))
#define cbi(reg,bit)	reg &= ~(BV(bit))
#define sbi(reg,bit)	reg |= (BV(bit))

// Prototype for the function we need to call from outside
void DynamixelPoll(void);

void DynamixelWrite(int id, int address, int value);
void DynamixelWriteByte(int id, int address, int value);
void DynamixelRead(int id, int address);
void DynamixelWriteBuf(unsigned char *buffer, int length);
void DynamixelWriteBuffer(int id, unsigned char *buffer, int length);
#endif
