// The basic idea is to always have a current input pin (currentInput)
// Which the user can change the output key for. The user can cycle
// Through the input pins, and once they have selected the pin they 
// want to change, they can cycle through the output pins.

int currentPin = 0;
void changeCurrentPin(int direction) {
  currentPin += direction;
  if (currentPin < 0) {
    currentPin = 0;
  }
  if (currentPin >= NUM_INPUTS) {
    currentPin = NUM_INPUTS - 1;
  }
}

// This changes the keycode for the currently selected pin
void changeCurrentPinKey(int direction) {
  int currentKeyIndex = indexForCode(keyCodes[currentPin]);
  
  currentKeyIndex += direction;
  if (currentKeyIndex < 0) {
    currentKeyIndex = 0;
  }
  if (currentKeyIndex >= NUM_ALL_KEYS) {
    currentKeyIndex = NUM_ALL_KEYS - 1;
  }
  
  keyCodes[currentPin] = keyCodeForIndex(currentKeyIndex);
}

void right() {
  changeCurrentPinKey(1); 
}
void left() {
  changeCurrentPinKey(-1);
}
void up() {
  changeCurrentPin(-1);
}
void down() {
  changeCurrentPin(1);
}


void printCurrentPin() {
  typeString(pinName(currentPin));
  typeString(" is mapped to ");
  typeString(keyNameForCode(keyCodes[currentPin]));
  typeString("\n");
}

