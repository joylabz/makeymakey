#include <avr/pgmspace.h>
#include "keynames.h"
#include "pinnames.h"

char buffer[30];

char *keyNameForIndex(int i) {
  strcpy_P(buffer, (char*)pgm_read_word(&(keyNames[i * 2 + 1]))); 
  return buffer;
}

int keyCodeForIndex(int i) {
  return (int)keyNames[i * 2];
}

char *keyNameForCode(int code) {
  for (int i = 0; i < NUM_ALL_KEYS; i++) {
    if (keyCodeForIndex(i) == code) {
     return keyNameForIndex(i);
    } 
  }
  return "NONE";
}

char *pinName(int pinnumber) {
  strcpy_P(buffer, (char*)pgm_read_word(&(pinNames[pinnumber])));
  return buffer;
}
