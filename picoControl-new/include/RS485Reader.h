#ifndef __RS485_READER_H__
#define __RS485_READER_H__

#include <Arduino.h>

#define RS485_BUFFER_SIZE (128) /* Just a lot so that it will not overflow */
#define BOARD_ID 13
#define RS485_SR 2

//int dynamixelIDs [] =       {   40,  21,   42,   43,   44,    45,   46};
//int dynamixelMaxPos [] =    {  260, 510, 2300, 1800, 1000,  1800, 1524}; 
//int dynamixelMinPos [] =    {  460, 340, 1900, 2200, 3000,  2200, 2596}; 
//int dynamixelOffset [] =    {    0,   0,    0,    0,    0,     0,    0};

//#define limit(number, min, max) (((number) > (max)) ? (max) : (((number) < (min)) ? (min) : (number)))
// processing and checking:

#define toggle(pin) digitalWrite(pin, !digitalRead(pin)) 
#define	outb(addr, data)	addr = (data)
#define	inb(addr)		(addr)
#define BV(bit)			(1<<(bit))
#define cbi(reg,bit)	reg &= ~(BV(bit))
#define sbi(reg,bit)	reg |= (BV(bit))

// Prototype for the function we need to call from outside
void RS485Poll(void);
void RS485Init(long bd, int RS485sr);
void RS485Write(int id, int address, int value);
void RS485WriteByte(int id, int address, int value);
void RS485Read(int id, int address);
void RS485WriteBuf(unsigned char *buffer, int length);
void RS485WriteBuffer(int id, unsigned char *buffer, int length);
#endif
