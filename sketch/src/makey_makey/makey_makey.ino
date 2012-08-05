/*
 ************************************************
 ************** MAKEY MAKEY *********************
 ************************************************
 
 /////////////////////////////////////////////////
 /////////////HOW TO EDIT THE KEYS ///////////////
 /////////////////////////////////////////////////
 - Edit keys in the settings.h file
 - that file should be open in a tab above (in Arduino IDE)
 - more instructions are in that file
 
 ////////////////////////////////////////////////
 //////// MaKey MaKey FIRMWARE v1.4 /////////////
 ////////////////////////////////////////////////
 by: Eric Rosenbaum, Jay Silver, and Jim Lindblom
 MIT Media Lab & Sparkfun
 start date: 2/16/2012 
 current release: 7/5/2012
 */

/////////////////////////
// DEBUG DEFINITIONS ////               
/////////////////////////
//#define DEBUG
//#define DEBUG2 
//#define DEBUG3 
//#define DEBUG_TIMING
//#define DEBUG_MOUSE
//#define DEBUG_TIMING2

////////////////////////
// DEFINED CONSTANTS////
////////////////////////

#define BUFFER_LENGTH    3     // 3 bytes gives us 24 samples
#define NUM_INPUTS       18    // 6 on the front + 12 on the back
//#define TARGET_LOOP_TIME 694   // (1/60 seconds) / 24 samples = 694 microseconds per sample 
//#define TARGET_LOOP_TIME 758  // (1/55 seconds) / 24 samples = 758 microseconds per sample 
#define TARGET_LOOP_TIME 744  // (1/56 seconds) / 24 samples = 744 microseconds per sample 

// id numbers for mouse movement inputs (used in settings.h)
#define MOUSE_MOVE_UP       -1 
#define MOUSE_MOVE_DOWN     -2
#define MOUSE_MOVE_LEFT     -3
#define MOUSE_MOVE_RIGHT    -4

// array of CPLEDs (charlieplexed LEDs)
#define NUM_CHARLIEPLEXED_LEDS   6
// convenience names for these LEDs (these better be <= NUM_CHARLIEPLEXED_LEDS-1)
#define CPLED_UP                 0
#define CPLED_DOWN               1
#define CPLED_LEFT               2
#define CPLED_RIGHT              3
#define CPLED_SPACE              4
#define CPLED_CLICK              5

#include "settings.h"
#include "test.h"
#include "common.h"
#include "charlie.h"

/////////////////////////
// STRUCT ///////////////
/////////////////////////
typedef struct {
  byte pinNumber;
  int keyCode;
  byte measurementBuffer[BUFFER_LENGTH]; 
  boolean oldestMeasurement;
  byte bufferSum;
  boolean pressed;
  boolean prevPressed;
  boolean isMouseMotion;
  boolean isMouseButton;
  boolean isKey;
} MakeyMakeyInput;

MakeyMakeyInput inputs[NUM_INPUTS];
CPLED charlieplexed_leds[NUM_CHARLIEPLEXED_LEDS];
byte cpled_states[NUM_CHARLIEPLEXED_LEDS];

///////////////////////////////////
// VARIABLES //////////////////////
///////////////////////////////////
int bufferIndex = 0;
byte byteCounter = 0;
byte bitCounter = 0;
int mouseMovementCounter = 0; // for sending mouse movement events at a slower interval

int pressThreshold;
int releaseThreshold;
boolean inputChanged;

int mouseHoldCount[NUM_INPUTS]; // used to store mouse movement hold data

// Pin Numbers
// input pin numbers for kickstarter production board
int pinNumbers[NUM_INPUTS] = {
  12, 8, 13, 15, 7, 6,     // top of makey makey board
  5, 4, 3, 2, 1, 0,        // left side of female header, KEBYBOARD
  23, 22, 21, 20, 19, 18   // right side of female header, MOUSE
};

// input status LED pin numbers
const int inputLED_a = 9;
const int inputLED_b = 10;
const int inputLED_c = 11;
const int outputK = 14;
const int outputM = 16;
byte ledCycleCounter = 0;

// timing
int loopTime = 0;
int prevTime = 0;
int loopCounter = 0;


///////////////////////////
// FUNCTION PROTOTYPES ////
///////////////////////////
void initialize_outputs(void);
void initializeArduino();
void initializeInputs();
void updateMeasurementBuffers();
void updateBufferSums();
void updateBufferIndex();
void updateInputStates();
void sendMouseButtonEvents();
void sendMouseMovementEvents();
void addDelay();
void cycleLEDs();
void danceLeds();
void updateOutLEDs();

//////////////////////
// SETUP /////////////
//////////////////////
void setup()
{
  initialize_outputs();
  debug_start();
  listen_for_debug();
  debug_end();
  danceLeds();
  initializeArduino();
  initializeInputs();
}

////////////////////
// MAIN LOOP ///////
////////////////////
void loop() 
{
  updateMeasurementBuffers();
  updateBufferSums();
  updateBufferIndex();
  updateInputStates();
  sendMouseButtonEvents();
  sendMouseMovementEvents();
  cycleLEDs();
  updateOutLEDs();
  addDelay();
}
           
//////////////////////////
// INITIALIZE ARDUINO
//////////////////////////
void initializeArduino() {
#ifdef DEBUG
  Serial.begin(9600);  // Serial for debugging
#endif

  /* Set up input pins 
   DEactivate the internal pull-ups, since we're using external resistors */
  for (int i=0; i<NUM_INPUTS; i++)
  {
    pinMode(pinNumbers[i], INPUT);
    digitalWrite(pinNumbers[i], LOW);
  }

#ifdef DEBUG
  delay(4000); // allow us time to reprogram in case things are freaking out
#endif

  Keyboard.begin();
  Mouse.begin();
}

void initialize_outputs(void) {
    // set up our charlieplexed LED structs
    charlieplexed_leds[CPLED_UP].vcc_pin = inputLED_b;
    charlieplexed_leds[CPLED_UP].ignore_pins[0] = inputLED_a;
    charlieplexed_leds[CPLED_UP].gnd_pin = inputLED_c;

    charlieplexed_leds[CPLED_DOWN].vcc_pin = inputLED_a;
    charlieplexed_leds[CPLED_DOWN].ignore_pins[0] = inputLED_c;
    charlieplexed_leds[CPLED_DOWN].gnd_pin = inputLED_b;
    
    charlieplexed_leds[CPLED_LEFT].vcc_pin = inputLED_b;
    charlieplexed_leds[CPLED_LEFT].ignore_pins[0] = inputLED_c;
    charlieplexed_leds[CPLED_LEFT].gnd_pin = inputLED_a;
    
    charlieplexed_leds[CPLED_RIGHT].vcc_pin = inputLED_c;
    charlieplexed_leds[CPLED_RIGHT].ignore_pins[0] = inputLED_a;
    charlieplexed_leds[CPLED_RIGHT].gnd_pin = inputLED_b;
    
    charlieplexed_leds[CPLED_SPACE].vcc_pin = inputLED_c;
    charlieplexed_leds[CPLED_SPACE].ignore_pins[0] = inputLED_b;
    charlieplexed_leds[CPLED_SPACE].gnd_pin = inputLED_a;
    
    charlieplexed_leds[CPLED_CLICK].vcc_pin = inputLED_a;
    charlieplexed_leds[CPLED_CLICK].ignore_pins[0] = inputLED_b;
    charlieplexed_leds[CPLED_CLICK].gnd_pin = inputLED_c;
    
    // initialize cpled state buffer
    cpled_states[CPLED_UP] = 0;
    cpled_states[CPLED_DOWN] = 0;
    cpled_states[CPLED_LEFT] = 0;
    cpled_states[CPLED_RIGHT] = 0;
    cpled_states[CPLED_SPACE] = 0;
    cpled_states[CPLED_CLICK] = 0;
    
    set_highz(inputLED_a);
    set_highz(inputLED_b);
    set_highz(inputLED_c);
   
    set_gnd(outputK);
    set_gnd(outputM);
}
  
///////////////////////////
// INITIALIZE INPUTS
///////////////////////////
void initializeInputs() {

  float thresholdPerc = SWITCH_THRESHOLD_OFFSET_PERC;
  float thresholdCenterBias = SWITCH_THRESHOLD_CENTER_BIAS/50.0;
  float pressThresholdAmount = (BUFFER_LENGTH * 8) * (thresholdPerc / 100.0);
  float thresholdCenter = ( (BUFFER_LENGTH * 8) / 2.0 ) * (thresholdCenterBias);
  pressThreshold = int(thresholdCenter + pressThresholdAmount);
  releaseThreshold = int(thresholdCenter - pressThresholdAmount);

#ifdef DEBUG
  Serial.println(pressThreshold);
  Serial.println(releaseThreshold);
#endif

  for (int i=0; i<NUM_INPUTS; i++) {
    inputs[i].pinNumber = pinNumbers[i];
    inputs[i].keyCode = keyCodes[i];

    for (int j=0; j<BUFFER_LENGTH; j++) {
      inputs[i].measurementBuffer[j] = 0;
    }
    inputs[i].oldestMeasurement = 0;
    inputs[i].bufferSum = 0;

    inputs[i].pressed = false;
    inputs[i].prevPressed = false;

    inputs[i].isMouseMotion = false;
    inputs[i].isMouseButton = false;
    inputs[i].isKey = false;

    if (inputs[i].keyCode < 0) {
#ifdef DEBUG_MOUSE
      Serial.println("GOT IT");  
#endif

      inputs[i].isMouseMotion = true;
    } 
    else if ((inputs[i].keyCode == MOUSE_LEFT) || (inputs[i].keyCode == MOUSE_RIGHT)) {
      inputs[i].isMouseButton = true;
    } 
    else {
      inputs[i].isKey = true;
    }
#ifdef DEBUG
    Serial.println(i);
#endif

  }
}

          
//////////////////////////////
// UPDATE MEASUREMENT BUFFERS
//////////////////////////////
void updateMeasurementBuffers() {

  for (int i=0; i<NUM_INPUTS; i++) {

    // store the oldest measurement, which is the one at the current index,
    // before we update it to the new one 
    // we use oldest measurement in updateBufferSums
    byte currentByte = inputs[i].measurementBuffer[byteCounter];
    inputs[i].oldestMeasurement = (currentByte >> bitCounter) & 0x01; 

    // make the new measurement
    boolean newMeasurement = digitalRead(inputs[i].pinNumber);

    // invert so that true means the switch is closed
    newMeasurement = !newMeasurement; 

    // store it    
    if (newMeasurement) {
      currentByte |= (1<<bitCounter);
    } 
    else {
      currentByte &= ~(1<<bitCounter);
    }
    inputs[i].measurementBuffer[byteCounter] = currentByte;
  }
}

///////////////////////////
// UPDATE BUFFER SUMS
///////////////////////////
void updateBufferSums() {

  // the bufferSum is a running tally of the entire measurementBuffer
  // add the new measurement and subtract the old one

  for (int i=0; i<NUM_INPUTS; i++) {
    byte currentByte = inputs[i].measurementBuffer[byteCounter];
    boolean currentMeasurement = (currentByte >> bitCounter) & 0x01; 
    if (currentMeasurement) {
      inputs[i].bufferSum++;
    }
    if (inputs[i].oldestMeasurement) {
      inputs[i].bufferSum--;
    }
  }  
}

///////////////////////////
// UPDATE BUFFER INDEX
///////////////////////////
void updateBufferIndex() {
  bitCounter++;
  if (bitCounter == 8) {
    bitCounter = 0;
    byteCounter++;
    if (byteCounter == BUFFER_LENGTH) {
      byteCounter = 0;
    }
  }
}

///////////////////////////
// UPDATE INPUT STATES
///////////////////////////
void updateInputStates() {
  inputChanged = false;
  for (int i=0; i<NUM_INPUTS; i++) {
    inputs[i].prevPressed = inputs[i].pressed; // store previous pressed state (only used for mouse buttons)
    if (inputs[i].pressed) {
      if (inputs[i].bufferSum < releaseThreshold) {  
        inputChanged = true;
        inputs[i].pressed = false;
        if (inputs[i].isKey) {
          Keyboard.release(inputs[i].keyCode);
        }
        if (inputs[i].isMouseMotion) {  
          mouseHoldCount[i] = 0;  // input becomes released, reset mouse hold
        }
      }
      else if (inputs[i].isMouseMotion) {  
        mouseHoldCount[i]++; // input remains pressed, increment mouse hold
      }
    } 
    else if (!inputs[i].pressed) {
      if (inputs[i].bufferSum > pressThreshold) {  // input becomes pressed
        inputChanged = true;
        inputs[i].pressed = true; 
        if (inputs[i].isKey) {
          Keyboard.press(inputs[i].keyCode);
        }
      }
    }
  }
#ifdef DEBUG3
  if (inputChanged) {
    Serial.println("change");
  }
#endif
}

/*
///////////////////////////
 // SEND KEY EVENTS (obsolete, used in versions with pro micro bootloader)
 ///////////////////////////
 void sendKeyEvents() {
 if (inputChanged) {
 KeyReport report = {
 0                                                        };
 for (int i=0; i<6; i++) {
 report.keys[i] = 0;
 } 
 int count = 0;
 for (int i=0; i<NUM_INPUTS; i++) {
 if (inputs[i].pressed && (count < 6)) {
 report.keys[count] = inputs[i].keyCode;
 
 #ifdef DEBUG3
 Serial.println(report.keys[count]);
 #endif
 
 count++;        
 }
 }
 if (count > 0) {
 report.modifiers = 0x00;
 report.reserved = 1;
 Keyboard.sendReport(&report);
 } 
 else {
 report.modifiers = 0x00;
 report.reserved = 0;
 Keyboard.sendReport(&report);
 }      
 } 
 else {
 // might need a delay here to compensate for the time it takes to send keyreport
 }
 }
 */

/////////////////////////////
// SEND MOUSE BUTTON EVENTS 
/////////////////////////////
void sendMouseButtonEvents() {
  if (inputChanged) {
    for (int i=0; i<NUM_INPUTS; i++) {
      if (inputs[i].isMouseButton) {
        if (inputs[i].pressed) {
          if (inputs[i].keyCode == MOUSE_LEFT) {
            Mouse.press(MOUSE_LEFT);
          } 
          if (inputs[i].keyCode == MOUSE_RIGHT) {
            Mouse.press(MOUSE_RIGHT);
          } 
        } 
        else if (inputs[i].prevPressed) {
          if (inputs[i].keyCode == MOUSE_LEFT) {
            Mouse.release(MOUSE_LEFT);
          } 
          if (inputs[i].keyCode == MOUSE_RIGHT) {
            Mouse.release(MOUSE_RIGHT);
          }           
        }
      }
    }
  }
}

//////////////////////////////
// SEND MOUSE MOVEMENT EVENTS
//////////////////////////////
void sendMouseMovementEvents() {
  byte right = 0;
  byte left = 0;
  byte down = 0;
  byte up = 0;
  byte horizmotion = 0;
  byte vertmotion = 0;

  mouseMovementCounter++;
  mouseMovementCounter %= MOUSE_MOTION_UPDATE_INTERVAL;
  if (mouseMovementCounter == 0) {
    for (int i=0; i<NUM_INPUTS; i++) {
#ifdef DEBUG_MOUSE
      Serial.println(inputs[i].isMouseMotion);  
#endif

      if (inputs[i].isMouseMotion) {
        if (inputs[i].pressed) {
          if (inputs[i].keyCode == MOUSE_MOVE_UP) {
            // JL Changes (x4): now update to 1 + a hold factor, constrained between 1 and mouse max movement speed
            up=constrain(1+mouseHoldCount[i]/MOUSE_RAMP_SCALE, 1, MOUSE_MAX_PIXELS);
          }  
          if (inputs[i].keyCode == MOUSE_MOVE_DOWN) {
            down=constrain(1+mouseHoldCount[i]/MOUSE_RAMP_SCALE, 1, MOUSE_MAX_PIXELS);
          }  
          if (inputs[i].keyCode == MOUSE_MOVE_LEFT) {
            left=constrain(1+mouseHoldCount[i]/MOUSE_RAMP_SCALE, 1, MOUSE_MAX_PIXELS);
          }  
          if (inputs[i].keyCode == MOUSE_MOVE_RIGHT) {
            right=constrain(1+mouseHoldCount[i]/MOUSE_RAMP_SCALE, 1, MOUSE_MAX_PIXELS);
          }  
        }
      }
    }

    // diagonal scrolling and left/right cancellation
    if(left > 0)
    {
      if(right > 0)
      {
        horizmotion = 0; // cancel horizontal motion because left and right are both pushed
      }
      else
      {
        horizmotion = -left; // left yes, right no
      }
    }
    else
    {
      if(right > 0)
      {
        horizmotion = right; // right yes, left no
      }
    }

    if(down > 0)
    {
      if(up > 0)
      {
        vertmotion = 0; // cancel vertical motion because up and down are both pushed
      }
      else
      {
        vertmotion = down; // down yes, up no
      }
    }
    else
    {
      if (up > 0)
      {
        vertmotion = -up; // up yes, down no
      }
    }
    // now move the mouse
    if( !((horizmotion == 0) && (vertmotion==0)) )
    {
      Mouse.move(horizmotion * PIXELS_PER_MOUSE_STEP, vertmotion * PIXELS_PER_MOUSE_STEP);
    }
  }
}

///////////////////////////
// ADD DELAY
///////////////////////////
void addDelay() {

  loopTime = micros() - prevTime;
  if (loopTime < TARGET_LOOP_TIME) {
    int wait = TARGET_LOOP_TIME - loopTime;
    delayMicroseconds(wait);
  }

  prevTime = micros();

#ifdef DEBUG_TIMING
  if (loopCounter == 0) {
    int t = micros()-prevTime;
    Serial.println(t);
  }
  loopCounter++;
  loopCounter %= 999;
#endif

}

///////////////////////////
// CYCLE LEDS
///////////////////////////
void cycleLEDs() {
  set_highz(inputLED_a);
  set_highz(inputLED_b);
  set_highz(inputLED_c);

  ledCycleCounter++;
    ledCycleCounter %= 6;

  if ((ledCycleCounter == 0) && inputs[0].pressed) {
    cpled_set(charlieplexed_leds[CPLED_UP], HIGH);
  }
  if ((ledCycleCounter == 1) && inputs[1].pressed) {
    cpled_set(charlieplexed_leds[CPLED_DOWN], HIGH);
  }
  if ((ledCycleCounter == 2) && inputs[2].pressed) {
    cpled_set(charlieplexed_leds[CPLED_LEFT], HIGH);
  }
  if ((ledCycleCounter == 3) && inputs[3].pressed) {
    cpled_set(charlieplexed_leds[CPLED_RIGHT], HIGH);
  }
  if ((ledCycleCounter == 4) && inputs[4].pressed) {
    cpled_set(charlieplexed_leds[CPLED_SPACE], HIGH);
  }
  if ((ledCycleCounter == 5) && inputs[5].pressed) {
    cpled_set(charlieplexed_leds[CPLED_CLICK], HIGH);
  }

}


///////////////////////////
// DANCE LEDS
///////////////////////////
void danceLeds()
{
  int delayTime = 50;
  int delayTime2 = 100;

  set_highz(inputLED_a);
  set_highz(inputLED_b);
  set_highz(inputLED_c);

  // CIRCLE
  for(int i=0; i<4; i++)
  {
    // UP
    cpled_set(charlieplexed_leds[CPLED_UP], HIGH);
    delay(delayTime);

    //RIGHT
    cpled_set(charlieplexed_leds[CPLED_RIGHT], HIGH);
    delay(delayTime);
   
    // DOWN
    cpled_set(charlieplexed_leds[CPLED_DOWN], HIGH);
    delay(delayTime);

    // LEFT
    cpled_set(charlieplexed_leds[CPLED_LEFT], HIGH);
    delay(delayTime    );    
  }    

  // WIGGLE
  for(int i=0; i<4; i++)
  {
    // SPACE
    cpled_set(charlieplexed_leds[CPLED_SPACE], HIGH);
    delay(delayTime2);    

    // CLICK
    cpled_set(charlieplexed_leds[CPLED_CLICK], HIGH);
    delay(delayTime2);    
  }
  
  set_highz(inputLED_a);
  set_highz(inputLED_b);
  set_highz(inputLED_c);
}

void updateOutLEDs()
{
  boolean keyPressed = 0;
  boolean mousePressed = 0;

  for (int i=0; i<NUM_INPUTS; i++)
  {
    if (inputs[i].pressed)
    {
      if (inputs[i].isKey)
      {
        keyPressed = 1;
#ifdef DEBUG
        Serial.print("Key ");
        Serial.print(i);
        Serial.println(" pressed");
#endif
      }
      else
      {
        mousePressed = 1;
      }
    }
  }

  if (keyPressed)
  {
    digitalWrite(outputK, HIGH);
    TXLED1;
  }
  else
  {
    digitalWrite(outputK, LOW);
    TXLED0;
  }

  if (mousePressed)
  {
    digitalWrite(outputM, HIGH);
    RXLED1;
  }
  else
  {
    digitalWrite(outputM, LOW);
    RXLED0;
  }
}






