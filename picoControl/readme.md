#### Readme.md ####

### V1.0 

These boards have been equipped with the PCA9685 which can do PWM low enough to drive servos directly. The 10 pin IDC socket next to the Pico has 8 GPIO pins of which 3 middle ones are also used for the incremental encoder

### V2.0

The V2.0 has additional 4-way JST sockets for hooking up additional IO (the same GPIO pins mentioned above) such as two H-bridges. 

Instead of the PCA9685 the PCA9635 is used which has a default 'HIGH' state so the Relays do not turn on during powerup. 

A L9110S driver has been added in the schematic to drive the high-side power for the relays, but this has no effect (and, in error, only allows one relay board to be powered simultanously. Apparently the state where both outputs are high does not exist and the component does not follow its datasheet :-(  )

