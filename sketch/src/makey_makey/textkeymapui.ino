// The basic idea is to always have a current input pin (currentInput)
// Which the user can change the output key for. The user can cycle
// Through the input pins, and once they have selected the pin they 
// want to change, they can cycle through the output pins.


// If you want the ui to erase itself, define this:
// #define BACKSPACE

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
  #ifdef BACKSPACE
  for (int i=0; i < 60; i++) {
    typeString("\b");
  }
  #endif
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
  }
}

void checkForPressedPins() {
  for (int i=0; i<NUM_INPUTS; i++) {
    if (!inputs[i].prevPressed && inputs[i].pressed) {
      pinPressed(i);
    }
  }
}

bool checkForShort() {
  // We are going to be reading from pin 5
  pinMode(pinNumbers[5], INPUT);
  digitalWrite(pinNumbers[5], LOW);


  // set pin 4 to be a sync, so if touched to 5, 5 will go low:
  pinMode(pinNumbers[4], OUTPUT);
  digitalWrite(pinNumbers[4], LOW);
  delay(1000);
  boolean result = !digitalRead(pinNumbers[5]); // should be 0

  // now set pin 4 to input with pull up resistors, this will make 5 go high, if connected
  // but it won't short anything ;-)
  pinMode(pinNumbers[4], INPUT);
  digitalWrite(pinNumbers[4], HIGH);
  delay(1000);
  result &= (digitalRead(pinNumbers[5])); // should be 1 now


  // Now we wait for you to let go
  
  if (result) {
    // Trick lights to go on 
    inputs[4].pressed = 1;
    inputs[5].pressed = 1; 
    cycleLEDs();
  // set pin 4 to be a sync, so if touched to 5, 5 will go low:
  pinMode(pinNumbers[4], OUTPUT);
  digitalWrite(pinNumbers[4], LOW);
  delay(1000);

    
    int starttime = micros();
    while(!digitalRead(pinNumbers[5])) {
      cycleLEDs();

      // wait
      delay(100);
      if (micros() - starttime > 15000000) {
        // WAITED TOO LONG, must be accidental!
        result = 0;
        break;
      }
    }
  }
  
  //reset pin4 back to input low
  pinMode(pinNumbers[4], INPUT);
  digitalWrite(pinNumbers[4], LOW);

  return result;
}
int reprogramming = 1;
void reprogramLoop() {
  reprogramming = checkForShort();

  if(reprogramming) {
    printCurrentPin();
    while (reprogramming == 1) {
      updateMeasurementBuffers();
      updateBufferSums();
      updateBufferIndex();
      updateInputStatesForReprogramming();
      cycleLEDs();
      updateOutLEDs();
      checkForPressedPins();
      addDelay();
    }
    // We need to re initialize with the new keycodes
    initializeInputs();
    danceLeds();
  }
}

void doneReprogramming() {
  writeKeyMap();
  reprogramming = 0;
  typeString("goodbye!");
}
