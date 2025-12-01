/*
 * SCSerial.h
 * hardware interface layer for waveshare serial bus servo
 * date: 2023.6.28
 */


#include "SCSerial.h"

SCSerial::SCSerial()
{
	IOTimeOut = 1; // was 100
	pSerial = NULL;
	    pinMode(TTL_SR, OUTPUT);
    digitalWrite(TTL_SR, TTL_RX);   // default to listen (HI-Z)
}

SCSerial::SCSerial(u8 End):SCS(End)
{
	IOTimeOut = 1;
	pSerial = NULL;
	    pinMode(TTL_SR, OUTPUT);
    digitalWrite(TTL_SR, TTL_RX);   // default to listen (HI-Z)
}

SCSerial::SCSerial(u8 End, u8 Level):SCS(End, Level)
{
	IOTimeOut = 1;
	pSerial = NULL;
	    pinMode(TTL_SR, OUTPUT);
    digitalWrite(TTL_SR, TTL_RX);   // default to listen (HI-Z)
}

int SCSerial::readSCS(unsigned char *nDat, int nLen)
{
	int Size = 0;
	int ComData;
	unsigned long t_begin = millis();
	unsigned long t_user;
	#ifdef SEND_ONLY
    digitalWrite(TTL_SR, TTL_RX);   // default to listen (HI-Z)
	while(1){
		ComData = pSerial->read();
		if(ComData!=-1){
			if(nDat){
				nDat[Size] = ComData;
			}
			Size++;
			t_begin = millis();
		}
		if(Size>=nLen){
			break;
		}
		t_user = millis() - t_begin;
		if(t_user>IOTimeOut){
			break;
		}
	}
	#endif
	return Size;
}

int SCSerial::writeSCS(unsigned char *nDat, int nLen)
{
    digitalWrite(TTL_SR, TTL_TX);   // default to transmit
	if(nDat==NULL){
		return 0;
	}
	int returnValue = pSerial->write(nDat, nLen);
	return returnValue;
}

int SCSerial::writeSCS(unsigned char bDat)
{
	    digitalWrite(TTL_SR, TTL_TX);   // default to transmit
	int returnValue = pSerial->write(&bDat, 1);
	return returnValue;
}

void SCSerial::rFlushSCS()
{
	digitalWrite(TTL_SR, TTL_RX);   // default to listen (HI-Z)
	while(pSerial->read()!=-1);
}

void SCSerial::wFlushSCS()
{
    // Wait until UART finished sending everything
    pSerial->flush();  
    
    // Optional tiny guard if you’re paranoid (at 1 Mbps, this is generous)
    // delayMicroseconds(5);

    // Back to receive mode
    digitalWrite(TTL_SR, TTL_RX);   // default to listen (HI-Z)
}