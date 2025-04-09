// RS485 on Serial 2 for communication
//
// knop_________kreef_______relay_______________________schildpad_________________vis______________
// 1  B4   (11) toeter    n B1                          toeter(varken)            toeter
// 2  B8   (3)  bibber    n A2                          links-rechts hoofd        staart
// 3  B2   (6)  bokken    n B2   //                     staart en zwemvliezen     ribben
// 4  B64  (12) schaarbek n B3   //                     bingomolen, ballenpomp    bovenvin
// 5  B128 (7)  spriet    t B4                          schild flappen            bubbels
// 6  B32  (10) nix       n A1 (schakel)                snottebel                 waaiers
// 7  A64  (15) spray     n B6                          spuit                     spuit
// 8  A128 (2)  staart    t B5                          diertjes                  zijvinnen
// 9  A32       klauwen   t B7 // moet sequence worden  nix                       nix
// *  A4   (14) klapbek   n A5                          bek                       bek
// 0  A8   (1)  poten     t A3                          zijpoten zwemmen          kettingzaag
// #  A2   (13) trilbek   n A4                          niets                     oogklap
// menu B1 (4)  lier up   n (motorcontroller)           omlaag
// up   B16(5)  lier down n (motercontroller)           OMHOOG
// down A1 (8)  arm uit   n A7                          niets                     niets
// ext  A16 (9) arm in    n A8                          niets                     niets
//
// 
//#define VIS (1)
#define KREEFT (1)
//#define SCHILDPAD (1)

// array with all relays, grouped on two connectors of 8 pins each
int relays[] =    {A8, A9, A10, A11, A12, A13, A14, A15, 40, 41,  42,  43,  44,  45,  46,  47};
int servoPins[] = {24, 25, 26, 27, 28, 29};
////// hardware specifics for animaltroniek wezens
#ifdef VIS
// toeter 40
// staartmotor A9
// ribben 41
// bovenvin 42
// bellenzuiger 43
// spuit 45
// zijvinne 44
// waaiers A8
// bek A12
// kettingzaag A10
// klaphoofd A11
// (rest leeg)
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
int servoMins[] =    { 60, 60, 60, 60, 40, 40};
int servoCenters[] = { 90, 90, 90, 90, 46, 46}; // voor zwaardvis
int servoMax[] =     {110, 110, 110, 110, 80, 80};

// een aparte functie is het knipoog. hier moet een knop voor worden gedefinieerd. Dus of een nummer tussen 0 en 32 (16 knopkanalen, kunnen dubbel)
#define BLINK_KEY ('1')

#endif

#ifdef KREEFT
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
int servoMins[] =    { 40, 40, 40, 40, 40, 40};
int servoCenters[] = { 90, 90, 90, 90, 46, 46}; // voor kreeft
int servoMax[] =     {130, 130, 130, 130, 90, 90};

// een aparte functie is het knipoog. hier moet een knop voor worden gedefinieerd. Dus of een nummer tussen 0 en 32 (16 knopkanalen, kunnen dubbel)
#define BLINK_KEY ('1')
#endif

#ifdef SCHILDPAD
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

