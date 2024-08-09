#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>

#include <DFRobot_DF1201S.h>
#include "SoftwareSerial.h"


void audioInit(DFRobot_DF1201S* player1,SoftwareSerial* player1port,DFRobot_DF1201S* player2,SoftwareSerial* player2port);


#endif