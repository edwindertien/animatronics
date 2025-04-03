// Original nunchuck:  signal:    ext. cable    black nunchuck:
// white -             ground     orange        white
// red -               3.3+v      black         red
// green -             data  A4   blue          blue
// yellow -            clock A5   red           green

#include <Wire.h>
#include <math.h>



class nunchuck {
public:
  float zeroAccelX = 128.0f;
  float zeroAccelY = 128.0f;
  float zeroAccelZ = 128.0f;
  byte data[6];
  byte analogStickX;
  byte analogStickY;
  byte buttons;
  float accelX;
  float accelY;
  float accelZ;

  void begin()
  {
    // Initialize and turn off data encryption
    Wire.begin();
    Wire.beginTransmission(0x52);
    Wire.write(0xf0);
    Wire.write(0x55);
    Wire.endTransmission();
    Wire.beginTransmission(0x52);
    Wire.write(0xfb);
    Wire.write(0x00);
    Wire.endTransmission();
  }

  void update(unsigned int delayMs)
  {

    Wire.beginTransmission(0x52);
    Wire.write(0x00);
    Wire.endTransmission();
    delayMicroseconds(delayMs);

    // Read in the data
    int i = 0;
    Wire.requestFrom(0x52, 6);
    while (Wire.available()) {
      data[i] = Wire.read();
      ++i;
    }

    if (i == 6) {
      analogStickX = data[0];
      analogStickY = data[1];
      accelX = static_cast<float>(data[2]) - zeroAccelX;
      accelY = static_cast<float>(data[3]) - zeroAccelY;
      accelZ = static_cast<float>(data[4]) - zeroAccelZ;
      buttons = 3 - (data[5] & 0x03);
    }
  }

  // Returns roll in range [-1.0, 1.0]
  float roll()
  {
    return atan2f(accelX, accelZ) / M_PI;
  }

  // Returns pitch in range [-0.5, 0.5]
  float pitch()
  {
    return atan2f(-accelY, sqrtf(accelX*accelX + accelZ*accelZ)) / M_PI;
  }
};
