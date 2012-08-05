#ifndef CHARLIE_H
#define CHARLIE_H

#define NUM_CHARLIE_OUTPUTS  3

typedef struct {
  byte vcc_pin;
  byte gnd_pin;
  byte ignore_pins[NUM_CHARLIE_OUTPUTS-2];
} CPLED;

#endif
