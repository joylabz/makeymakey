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





void pinPressed(int p) {
  switch (p) {
    case 0:
      up();
      break;
    case(1):
      down();
      break;
    case(2):
      left();
      break;
    case(3):
      right();
      break;
    case(4): // space... Also click, because they are shorted together ;-:
      doneReprogramming();
      break;
  }
  printCurrentPin();
}

void updateInputStatesForReprogramming() {
  inputChanged = false;
  
  for (int i=0; i<NUM_INPUTS; i++) {
    inputs[i].prevPressed = inputs[i].pressed; // store previous pressed state (only used for mouse buttons)
    if (inputs[i].pressed) {
      if (inputs[i].bufferSum < releaseThreshold) {  
        inputs[i].pressed = false;
      }
    }   
    else if (!inputs[i].pressed) {
      if (inputs[i].bufferSum > pressThreshold) {  // input becomes pressed
        inputs[i].pressed = true; 
      }
    }
    if (!inputs[i].prevPressed && inputs[i].pressed) {
      pinPressed(i);
    }
  }
}

bool checkForShort() {
  pinMode(pinNumbers[4], OUTPUT);
  digitalWrite(pinNumbers[4], HIGH);
  boolean result = digitalRead(pinNumbers[5]);
  digitalWrite(pinNumbers[4], LOW);
  result = result & (!digitalRead(pinNumbers[5]));
  digitalWrite(pinNumbers[4], HIGH);
  result = result & digitalRead(pinNumbers[5]);

// reset pin as input
  pinMode(pinNumbers[4], INPUT);
  digitalWrite(pinNumbers[4], LOW);

  return 1;
}

int reprogramming = 1;
void reprogramLoop() {
//  reprogramming = checkForShort();
  while (reprogramming) {
    updateMeasurementBuffers();
    updateBufferSums();
    updateBufferIndex();
  
    updateInputStatesForReprogramming();
    addDelay();
  }
}

void doneReprogramming() {
  writeKeyMap();
  reprogramming = 0;
}
