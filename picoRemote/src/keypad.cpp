#include "keypad.h"

keypad::keypad(int *pins, int size) {
    _pins = pins;
    _numPins = size;
}

void keypad::begin() {
    for (int i = 3; i < 7; i++) {
        pinMode(_pins[i], INPUT_PULLUP);
    }
}

char keypad::getKeyValue(unsigned int value) {
    char keys[] = {
        '1', '4', '7','*',
        '2', '5', '8','0',
        '3', '6', '9','#'
       
    };
    for (int i = 0; i < 12; i++) {
        if (value == 1 << i) {
            return keys[i];
        }
    }
    return 0;
}

unsigned int keypad::getKeypad() {
    unsigned int output = 0;
    for (int i = 0; i < 3; i++) {
        pinMode(_pins[i], OUTPUT);
        pinMode(_pins[(i + 1) % 3], INPUT);
        pinMode(_pins[(i + 2) % 3], INPUT);
        delayMicroseconds(1);
        output += ((digitalRead(_pins[3]) + 2 * digitalRead(_pins[4]) + 4 * digitalRead(_pins[5]) + 8 * digitalRead(_pins[6])) << (i * 4));
    }
    return 4095 - output;
}