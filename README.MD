# Animatronics
## intro
In this archive you will find mostly RPI pico based projects for remote control of animatronics and theatre. 

## designs
Currently there are three physical designs:


- the [picoArtnetDMX](/picoArtnetDMX/picoArtnetDMX)  unit which connects to artnet (ethernet) or WIFI and sends DMX data
- the [picoControl](/picoControl) module which is designed to interface with different radio modules, links to RS485/dynamixel, has two sound modules and expansion ports for Relays and / or Servos (using PCF8596)
- the [picoServo](/picoServo) node, a very small board with RS485, -just- three input and three output ports, power supply and grove I2C socket
- the [picoRemote](/picoRemote) contains a 4067 mux to allow 16 analog channels (joysticks, buttons, poteniometers), OLED display and radio module of choice. 

- the [picoMiniMIDIcontrol](/picoMiniMIDIcontrol/) uses a grove 8-knob module and a pico to act as a very basic USB-midi control panel. It has been used as complementary input device for the picoRemote device
- the [picoMIDInode](/picoMIDInode/) uses the picoServo board as USB-MIDI host (with a USB-A socket wired to the 'servo' output pins) and acts as USB-MIDI device


Todo: check scalability of images: [markdown cheatsheet](https://www.markdownguide.org/cheat-sheet/)
![pico Artnet PCB](images/picoArtnet.jpg)