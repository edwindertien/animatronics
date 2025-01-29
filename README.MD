# Animatronics
## intro
In this archive you will find mostly RPI pico based projects for remote control of animatronics and theatre.
Currently there are three physical designs:

- the picoArtnetDMX unit which connects to artnet (ethernet) or WIFI and sends DMX data
- the picoControl module which is designed to interface with different radio modules, links to RS485/dynamixel, has two sound modules and expansion ports for Relays and / or Servos (using PCF8596)
- the picoServo node, a very small board with RS485, -just- three input and three output ports, power supply and grove I2C socket
- the picoRemote