#include "common.h"

void set_vcc(byte pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
}

void set_gnd(byte pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void set_highz(byte pin) {
    pinMode(pin, INPUT);  
    digitalWrite(pin, HIGH);
}
