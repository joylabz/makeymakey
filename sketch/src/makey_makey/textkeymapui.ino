int currentInput = 0;
void changeCurrentInput(int direction) {
  currentInput += direction;
  if (currentInput < 0) {
    currentInput = 0;
  }
  if (currentInput >= NUM_INPUTS) {
    currentInput = NUM_INPUTS - 1;
  }
}
void changeCurrentInputValue(int direction) {
  int currentKeyIndex = indexForCode(keyCodes[currentInput]);
  
  currentKeyIndex += direction;
  if (currentKeyIndex < 0) {
    currentKeyIndex = 0;
  }
  if (currentKeyIndex >= NUM_ALL_KEYS) {
    currentKeyIndex = NUM_ALL_KEYS - 1;
  }
  
  keyCodes[currentInput] = keyCodeForIndex(currentKeyIndex);
}

void right() {
  changeCurrentInputValue(1); 
}
void left() {
  changeCurrentInputValue(-1);
}
void up() {
  changeCurrentInput(-1);
}
void down() {
  changeCurrentInput(1);
}

