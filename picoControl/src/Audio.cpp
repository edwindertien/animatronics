
#include "Audio.h"

void audioInit(DFRobot_DF1201S* player1,SoftwareSerial* player1port,DFRobot_DF1201S* player2,SoftwareSerial* player2port){
  player1port->begin(115200);
  player2port->begin(115200);

  player1port->listen();
  while (!player1->begin(*player1port)) {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }
  player2port->listen();
  while (!player2->begin(*player2port)) {
    Serial.println("Player 2 init failed, please check the wire connection!");
    delay(1000);
  }
  
  player1port->listen();
  player1->switchFunction(player1->MUSIC);
  player1->setPrompt(false);
  player1->setPlayMode(player1->SINGLECYCLE);
  player1->setVol(0);
  player1->playFileNum(/*File Number = */ 0);
  delay(10);
  player1->pause();
  delay(10);
  player1->playFileNum(/*File Number = */ 0);
  delay(10);
   player1->pause();
  // transducer
  player2port->listen();
  player2->switchFunction(player2->MUSIC);
  player2->setPlayMode(player2->SINGLE);
  player2->setPrompt(false);
  player2->setVol(0);
  player2->playFileNum(/*File Number = */ 18);
  player2->setVol(0);
  delay(10);
  player2->pause();
  delay(10);
  player2->playFileNum(/*File Number = */ 18);
  player2->setVol(0);
  delay(10);
   player2->pause();




  // player1port->listen();
  // player1->switchFunction(player1->MUSIC);
  // player1->setPrompt(false);
  // player1->setPlayMode(player1->SINGLE);
  // player1->setVol(/*VOL = */ 20);
  // player2port->listen();
  // player2->switchFunction(player2->MUSIC);
  // player2->setPlayMode(player2->SINGLE);
  // player2->setPrompt(false);
  // player2->setVol(/*VOL = */ 20);

}