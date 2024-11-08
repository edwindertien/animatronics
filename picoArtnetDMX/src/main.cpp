/*
  Artnet gateway for DMX using Pico
  // inspiration from https://www.techandtransit.com/lighting/pi-pico-artnet-to-dmx-node/
  https://github.com/tmingos/Pico-ArtNet-DMX-Node/tree/main
  https://docs.wiznet.io/Product/iEthernet/W5100S/w5100s-evb-pico
  https://forums.raspberrypi.com/viewtopic.php?t=22242

  https://community.element14.com/technologies/open-source-hardware/b/blog/posts/dmx-explained-dmx512-and-rs-485-protocol-detail-for-lighting-applications

*/
#include <Arduino.h>
#include <DmxOutput.h>  // comment/uncomment #define USE_UARTx in lib_dmx.h as needed
#include <ArtnetEther.h>

#include <Ethernet.h>

// Ethernet stuff
const IPAddress ip(192, 168, 68, 84);

ArtnetReceiver artnet;
uint16_t universe1 = 1;  // 0 - 32767
uint8_t universe2 = 2;   // 0 - 15

// Declare an instance of the DMX Output
DmxOutput dmx;

// Create a universe that we want to send.
// The universe must be maximum 512 bytes + 1 byte of start code
#define UNIVERSE_LENGTH 512
volatile uint8_t universe[UNIVERSE_LENGTH + 1];



#include <Wire.h>
#include <Adafruit_GFX.h>      // graphics, drawing functions (sprites, lines)
#include <Adafruit_SSD1306.h>  // display driver
#define SCREEN_WIDTH 128       // OLED display width, in pixels
#define SCREEN_HEIGHT 64       // OLED display height, in pixels
#define OLED_RESET -1          // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C    ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
unsigned int localPort = 6454;  // local port to listen on
int artNetTimeOut = 0;
int frameCount = 0;
void callback(const uint8_t *data, uint16_t size, const ArtDmxMetadata &metadata, const ArtNetRemoteInfo &remote) {
  // you can also use pre-defined callbacks
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);


  if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }


  oled.clearDisplay();               // start the screen
  oled.setTextSize(1);               // Normal 1:1 pixel scale
  oled.setTextColor(SSD1306_WHITE);  // Draw white text
  oled.setCursor(0, 0);
  oled.println(F("Artnet Pico Hub"));
  oled.display();

  Ethernet.init(17);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  // print your local IP address:
  Serial.print("DHCP IP address: ");
  Serial.println(Ethernet.localIP());
  oled.setCursor(0, 10);
  oled.println(Ethernet.localIP());
  oled.display();

  delay(1000);
  artnet.begin();

  // if Artnet packet comes to this universe, this function (lambda) is called
  artnet.subscribeArtDmxUniverse(universe1, [&](const uint8_t *data, uint16_t size, const ArtDmxMetadata &metadata, const ArtNetRemoteInfo &remote) {
    if (digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN, LOW);
    else digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("lambda : artnet data from ");
    Serial.print(remote.ip);
    Serial.print(":");
    Serial.print(remote.port);
    Serial.print(", universe = ");
    Serial.print(universe1);
    Serial.print(", size = ");
    Serial.print(size);
    Serial.print(") :");
    
    for (size_t i = 1; i < size + 1; i++) {
      universe[i] = data[i];
      Serial.print(data[i]);
      Serial.print(",");
    }
    artNetTimeOut = 0;
    frameCount++;
    Serial.println();
  });

}

 // end setup()

void loop() {
  static unsigned long screentimer;
  artnet.parse();  // check if artnet packet has come and execute callback
  if (millis() > screentimer + 99) {
    screentimer = millis();
    if(artNetTimeOut<100) artNetTimeOut++;
    oled.clearDisplay();               // start the screen
    oled.setCursor(0, 0);
    oled.print(F("IP:"));
    oled.println(Ethernet.localIP());
    if(artNetTimeOut>5) oled.println(F("no data"));
    else oled.println(frameCount);
    frameCount = 0;
    for (int n = 0; n < 32; n++) {
      oled.fillRect(n * 6, 32 - universe[n] / 16, 4, universe[n] / 16, SSD1306_INVERSE);
    }
    oled.display();
  }
}
void setup1(){
  dmx.begin(0, pio0);
}

void loop1(){
    dmx.write(universe, 32);
  while (dmx.busy()) { /* Do nothing while the DMX frame transmits */
  }
}