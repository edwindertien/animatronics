/*****************************************************************************
   Dynamixel controlled servo board

  // 1 blink
  // 2 eyes eyes up-down
  // 3 eyes left-right
  // 4 jaw
  // 5 mouth corner left
  // 6 mouth corner right
  // 7 toes left
  // 8 toes right

 *********************************************************************/
#include "Arduino.h"
#include <Servo.h>
#include "DynamixelReader.h"
#include "eyeState.h"
//#include <avr/wdt.h>

#include <Adafruit_NeoPixel.h>
// How many NeoPixels are attached to the Arduino?
#define NODES 9
#define LED_COUNT NODES * 37
#define LED_PIN 4 //gp4, SDA pin on GROVE
// Declare our NeoPixel strip object:

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
#include <avr/pgmspace.h>  // to store the patterns in flash

int steps[NODES] = { 12, 12, 12, 12, 12, 12, 12, 12, 12 };
int nextfire[NODES] = { 50, 80, 50, 50, 50, 50, 50, 50, 50 };


//// prototypes 
void startServo(int n);
void stopServo(int n);
void stopServos(void);
void updatePattern(void);

void showImage(int , int* , int , int , int x);
uint32_t Wheel(byte, int );


//////////////////   x,  y, lid,   m L,  m R,
int servoMin[] { 1100, 1500, 1300, 1400, 1100, 1100};
int servoCenter[] { 1500, 1580, 1800, 1500, 1500, 1500};
int servoMax[] { 1900, 1500, 1800, 1600, 1900, 1900};

#define NUMSERVOS 3

/////////////// WinAVR defines /////////////////////////
#define toggle(pin) digitalWrite(pin, !digitalRead(pin))
/////////////////////////////////////////////////////////

Servo myServo[NUMSERVOS];

int servoPins[] = {
  6,7,8
};

unsigned long loopTime;


/* LEDS */

#define LED_GREEN 13

void setup() {
   Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SS, OUTPUT);
  // communication
  Serial1.begin(1000000);
  rp2040.wdt_begin(250);  // alternative for arduino wdt

 
  //startServo(2);
  //startServo(1);
  //startServo(0);
}

void setup1(){
 strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(255);  // Set BRIGHTNESS to about 1/5 (max = 255)

}

void loop1(){
    static unsigned long patternTimer;
if(millis() > patternTimer + 49) {

    patternTimer = millis();
    updatePattern();
  }
}

void loop() {


  DynamixelPoll();
  rp2040.wdt_reset();  // pat the dog
  if (millis() > loopTime + 49) {
    loopTime = millis();
    nudgeTimeOut(); // take care of message timeout.. 

  
    if (getTimeOut() > 20)  // safe values!!
    {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("no data");
      stopServos();
    }
  }

}




/*************************************************************************
   Given an ID and the data, it determines what to do (and does it)
   If we're here, we already know that the ID is 'useful', ie, it is the
   ID of a motor that is controlled by this board.
*/
boolean enabled[NUMSERVOS];


volatile int global_brightness = 10;
volatile int global_speed = 200;

void ProcessDynamixelData(const unsigned char ID, const int dataLength, const unsigned char* const Data) {
  if (ID == BOARD_ID) {
    toggle(LED_BUILTIN);
    

    if (Data[0] == 0x03) {  // dynamixel write
      if(Data[8]>1){
      if (!enabled[0])startServo(0);
      //if (!enabled[1])startServo(1);
      if (!enabled[2])startServo(2);
      myServo[0].writeMicroseconds(map(Data[1], 0, 255, servoMin[0], servoMax[0]));
      //myServo[1].writeMicroseconds(map(Data[3], 0, 255, servoMin[2], servoMax[2]));
      myServo[2].writeMicroseconds(map(Data[3], 0, 255, servoMax[2], servoMin[2]));
      }
      else stopServos();
      global_brightness = map(Data[4], 0, 255, 0, 50);
      global_speed = map(Data[5], 0, 255, 300, 5);
    }
  }
}

void startServo(int n) {
  if (n < NUMSERVOS) {
    myServo[n].attach(servoPins[n], servoMin[n], servoMax[n]);
    myServo[n].writeMicroseconds(servoCenter[n]);
    enabled[n] = true;
  }
}

void stopServos() {
  for (int n = 0; n < NUMSERVOS; n++) {
    stopServo(n);
  }
}
void stopServo(int n) {
 // Serial.println("stopping");
  if (n < NUMSERVOS) {
    myServo[n].detach();
    enabled[n] = false;
  }
}



void updatePattern(){
  static int k;
  static int sinvalue;
       // now for the pattern stuff 
    k++;
    if (k > 255) k = 0;
    for (int i = 0; i < NODES; i++) {
      if (nextfire[i] == 0) {
        steps[i] = 12;
      }
      sinvalue = 40 * sin(6.28 * ((k + 20 * i) % 256) / 256.0);
      showImage(i, eyeState[13], 127 + sinvalue, global_brightness, 1);
      if (steps[i] > 0) showImage(i, eyeState[12 - steps[i]], 200, global_brightness, 1);
      if (steps[i] > 0) {
        steps[i]--;
        nextfire[i] = random(5, global_speed); //firetimes[i];//10 + 10 * i;
      }
      if (nextfire[i] > 0) nextfire[i]--;
    }
    strip.show();
}

void showImage(int node, int* pointer, int color, int brightness, int x) {
  // Fill along the length of the strip in various colors...
  for (int i = 0; i < 37; i++) {
    if (pointer[i] == 1) {

      strip.setPixelColor(node * 37 + i, Wheel(color, brightness));
    }
    else {
      if (x == 0) strip.setPixelColor(node * 37 + i, Wheel(0, 0));
    }
  }
}

uint32_t Wheel(byte WheelPos, int brightness) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color((brightness / 256.0) * (255 - WheelPos * 3), 0, (brightness / 256.0) * WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, (brightness / 256.0) * WheelPos * 3, (brightness / 256.0) * (255 - WheelPos * 3));
  }
  WheelPos -= 170;
  return strip.Color((brightness / 256.0) * (WheelPos * 3), (brightness / 256.0) * (255 - WheelPos * 3), 0);
}
