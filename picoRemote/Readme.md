### PicoRemote

This remote (board and source) have the following features:
 - CRSF or (own) APC220 RF protocol
 - 16 channel analogue mux
 - I2C Oled display
 - (optional) Wii Nunchuck (I2C)
 - (optional) 12-key keypad

 for every model a different channel map is given

in order to reduce data load, for switch channels 'stuffing' is possible, where every switch channel 
(either single or double switch) occupies 2 bits. (so 32 bits for 16 (double) switches)
the keypad is sent as one single character ('0', '1', .. ,'9', '#', '*')

note that the wiring coulour for the 3axis joystick is counter-intuitive: 

black: signal
white: GND
red: VCC

Every channel is wired with a standard 'servo' cable: 

brown: GND
red: VCC
orange: signal

Note that the ELRS transmitter (starting version 3.4.3) has to be put into 16ch/2 switch mode. This only works with 100 or 333Hz. 

a section for bi-directional communication has been designed, however not implemented yet (channel 15 and 16 give Vbat and RSSI back?)

in order to bypass this ciruit a small solder jumper is necessary on the bottom of the PCB

For every channel pull up and pull down resistors are mounted in 8 x 4 channel SIL array of 4x 10k. 