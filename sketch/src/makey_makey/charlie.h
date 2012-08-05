#ifndef CHARLIE_H
#define CHARLIE_H

#define NUM_CHARLIE_OUTPUTS  3

typedef struct {
  byte vcc_pin;
  byte gnd_pin;
  byte ignore_pins[NUM_CHARLIE_OUTPUTS-2]; // the pins we want to set high-z (should be number of charlie outputs - 1 (gnd_pin) - 1 (vcc_pin)
} CPLED;

#endif
