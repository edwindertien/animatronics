// RS485 on Serial 2 for communication
//
//#define VIS (1)
//#define KREEFT (1)
//#define SCHILDPAD (1)
#define ANIMAL_LOVE (1)
//////////////////////////////////////////////////////////////////////////////////////////////
// array with all relays, grouped on two connectors of 8 pins each
#ifdef ANIMAL_LOVE
int relays[] =    { 40, 41,  42,  43,  44,  45,  46,  47,A8, A9, A10, A11, A12, A13, A14, A15};
#else
int relays[] =    {A8, A9, A10, A11, A12, A13, A14, A15,40, 41,  42,  43,  44,  45,  46,  47};
#endif
int servoPins[] = {24, 25, 26, 27, 28, 29};
//////////////////////////////////////////////////////////////////////////////////////////////
////// hardware specifics for animaltroniek wezens specifiek
#ifdef VIS
#define RS485_BAUD 57600

// RELAY (2 groups, 0..7 and 8..15)
// r0 oorvinnen   
// r1 staart   
// r2 kettingzaag 
// r3 oogklap, 
// r4 klapbek
// r8 bel
// r9 ribben
// r10 bovenvin
// r11 bubbelfles  
// r12 zijvinnen
// r13 water 
//
// map to keypad ('0' tm '9' and 4 character array [] [] [] [])
#define NUM_ACTIONS 6
Action myActionList[NUM_ACTIONS] = {
  Action('1', 3, DIRECT),    // keypad 1 
  Action(0, 10, DIRECT, 12, 0, 9 ),  // bovenvin, zijvin, oorvin, ribben
  Action(4, 11, DIRECT), // bubbelfles
  Action(2, 1, DIRECT,2), // staart
  Action(8, 13,DIRECT), // water
  Action(9, 8, DIRECT, 4) // bel, bek (bubbel)

};
// hier in deze lijstjes staan de minium en maximum
// servo posities. Een servo kan normaal bewegen
// tussen 0 en 180 graden. (van 40-130 geeft een 90 graden bereik)
// x: links/rechts, y: omhoog/beneden, l: ooglid
// voor de kreeft moet de volgende regel 'code' zijn, voor
// de vis moet je hem uitcommentariereen. (dus // er voor zetten)
int servoMins[] =    { 60, 60, 60, 60, 40, 40};
int servoCenters[] = { 90, 90, 90, 90, 46, 46}; // voor zwaardvis
int servoMax[] =     {110, 110, 110, 110, 80, 80};

// een aparte functie is het knipoog. hier moet een knop voor worden gedefinieerd. Dus of een nummer tussen 0 en 32 (16 knopkanalen, kunnen dubbel)
#define BLINK_KEY ('2')

#endif

#ifdef KREEFT
#define RS485_BAUD 57600

// [0] ratel  40
// [1] tril achter  A9
// [2] bokken  41
// [3] bek in uit  42
// [4] sprietmotor  43
// [5] klauw  A8
// [6] kleine spuit 45
// [7] staart 44
// [8] leeg 46
// [9] klapbekje A12
// [10] achterpoten A10
// [11] tril voor A11
// [12] leeg 47
// [13] leeg A13
// [14] klauw uit A14
// [15] klauw in A15
// important mapping of actions, buttons, relay channels and sounds
#define NUM_ACTIONS 9
Action myActionList[NUM_ACTIONS] = {
//  Action('a', -1, DIRECT, &tandkrans, 100, "/bubble.mp3", &player1),
Action(0,11 ,DIRECT,12,2),
Action(8,8,DIRECT),
Action(9,5,DIRECT),
Action(13,6,DIRECT,0), // armen in 
Action(12,7,DIRECT,0), // armen uit
Action(15,9,DIRECT),
Action(14,10,DIRECT),
Action('1',4,DIRECT),
Action('3',3,DIRECT,1),


};

/*
r0 klauw
r1 tril achter
r2 poten + achterflap
r3 trilbek
r4 klapbek
r5 spuit (stuk)
r6 arm uit
r7 arm in
r8 toeter (ratel)
r9 bokken
r10 schuifbek
r11 voelsprieten
r12 achterflap hoog

*/


// hier in deze lijstjes staan de minium en maximum
// servo posities. Een servo kan normaal bewegen
// tussen 0 en 180 graden. (van 40-130 geeft een 90 graden bereik)
// x: links/rechts, y: omhoog/beneden, l: ooglid
// voor de kreeft moet de volgende regel 'code' zijn, voor
// de vis moet je hem uitcommentariereen. (dus // er voor zetten)
int servoMins[] =    { 40, 40, 40, 40, 40, 40};
int servoCenters[] = { 90, 90, 90, 90, 46, 46}; // voor kreeft
int servoMax[] =     {130, 130, 130, 130, 90, 90};

// een aparte functie is het knipoog. hier moet een knop voor worden gedefinieerd. Dus of een nummer tussen 0 en 32 (16 knopkanalen, kunnen dubbel)
#define BLINK_KEY ('2')
#endif

#ifdef ANIMAL_LOVE
#define RS485_BAUD 9600

#define NUM_ACTIONS 8
Action myActionList[NUM_ACTIONS] = {
Action(5, 0, DIRECT),  //water
Action(6, 1, DIRECT),  //blaas
Action(8, 2, DIRECT),  //L+R
Action(10, 3, DIRECT), // BEK
Action(12, 4, DIRECT), // knik
Action('6', 5, DIRECT),
Action('7', 6, DIRECT),
Action('8', 7, DIRECT),
};

int servoMins[] =    { 40, 40, 40, 40, 40, 40};
int servoCenters[] = { 90, 90, 90, 90, 90, 90}; // voor zwaardvis
int servoMax[] =     {140, 140, 140, 140, 130, 130};

#endif

#ifdef SCHILDPAD
#define RS485_BAUD 57600

// varkentje toeter 40
// hoofd lr A9
// flipper 41
// eieren 42
// schild 43
// spuit 45
// diertjes 44
// bek A12
// snotpomp A8
// zijvinnn  A10
// extra spuitrelay A11

#define NUM_ACTIONS 6
Action myActionList[NUM_ACTIONS] = {
//  Action('a', -1, DIRECT, &tandkrans, 100, "/bubble.mp3", &player1),
  Action('1', 0, DIRECT, 5,7),
  Action('2', 1, DIRECT),
  Action('3', 2, DIRECT),
  Action('#', 3, DIRECT),
  Action(8, 4, DIRECT),
  Action(9, 5, DIRECT),
};
// hier in deze lijstjes staan de minium en maximum
// servo posities. Een servo kan normaal bewegen
// tussen 0 en 180 graden. (van 40-130 geeft een 90 graden bereik)
// x: links/rechts, y: omhoog/beneden, l: ooglid
// voor de kreeft moet de volgende regel 'code' zijn, voor
// de vis moet je hem uitcommentariereen. (dus // er voor zetten)
int servoMins[] =    { 60, 60, 60, 60, 45, 45};
int servoCenters[] = { 90, 90, 90, 90, 46, 46}; // voor zwaardvis
int servoMax[] =     {110, 110, 110, 110, 80, 80};

// een aparte functie is het knipoog. hier moet een knop voor worden gedefinieerd. Dus of een nummer tussen 0 en 32 (16 knopkanalen, kunnen dubbel)
#define BLINK_KEY ('1')
#endif



