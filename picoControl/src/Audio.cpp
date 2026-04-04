#include "vehicle_select.h"
#include "config.h"
#include "Audio.h"

#if USE_AUDIO >= 1
void audioInit() {
    player1port.begin(115200);
    player1port.listen();
    while (!player1.begin(player1port)) {
        Serial.println("Player 1 init failed, check wiring!");
        delay(1000);
    }
    player1.switchFunction(player1.MUSIC);
    player1.setPrompt(false);
    player1.setPlayMode(player1.SINGLECYCLE);
    player1.setVol(0);
    player1.playFileNum(1);
    delay(10);
    player1.pause();
    delay(10);
    player1.playFileNum(1);
    delay(10);
    player1.pause();

#if USE_AUDIO >= 2
    player2port.begin(115200);
    player2port.listen();
    while (!player2.begin(player2port)) {
        Serial.println("Player 2 init failed, check wiring!");
        delay(1000);
    }
    player2.switchFunction(player2.MUSIC);
    player2.setPlayMode(player2.SINGLE);
    player2.setPrompt(false);
    player2.setVol(0);
    player2.playFileNum(1);
    player2.setVol(0);
    delay(10);
    player2.pause();
    delay(10);
    player2.playFileNum(1);
    player2.setVol(0);
    delay(10);
    player2.pause();
#endif // USE_AUDIO >= 2
}
#endif // USE_AUDIO >= 1