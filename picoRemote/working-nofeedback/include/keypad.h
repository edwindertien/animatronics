#ifndef KEYPAD_H
#define KEYPAD_H

#include <Arduino.h>

class keypad {
public:
    // Constructor
    keypad(int *pins, int size);

    // Initialize the keypad
    void begin();

    // Get the value of the pressed key (if any)
    char getKeyValue(unsigned int value);

    // Scan the keypad and return the pressed key value
    unsigned int getKeypad();

private:
    int* _pins;         // Pins array
    int _numPins;       // Number of keypad pins
};

#endif