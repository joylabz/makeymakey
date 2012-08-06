#include "charlie.h"
#include "common.h"
    
void cpled_set(CPLED led, boolean state) {
  // put any pins we don't need into a high-z state
  for (byte i; i < sizeof(led.ignore_pins); i++) {
    set_highz(led.ignore_pins[i]);
  }
  if (state) {
    set_gnd(led.gnd_pin);
    set_vcc(led.vcc_pin);
  }
  else {
    set_gnd(led.vcc_pin);
  }
}
