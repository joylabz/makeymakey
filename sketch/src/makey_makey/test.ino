#include "test.h"

int testpin_map[24];

boolean test_pin(int toggle_pin, int sense_pin) {
  boolean return_val = true;
  
  // reset both pins
  set_highz(sense_pin);
  set_highz(toggle_pin);
    
  // put sense_pin in input mode
  // no internal pullup because we have externals on the board
  pinMode(sense_pin, INPUT);
  digitalWrite(sense_pin, LOW);
  
  delay(2); // wait for pins to settle

  // verify sense pin is HIGH
  return_val &= ( digitalRead(sense_pin) == HIGH );
  
  // pull toggle pin low
  set_gnd(toggle_pin);
  
  delay(2); // wait for pins to settle
  
  // verify that all other inputs are STILL HIGH!
  // if they're not, this could indicate a short
  for (int i=0; i<NUM_INPUTS; i++) {
    int otherPin = pinNumbers[i];
    if (otherPin == toggle_pin || otherPin == sense_pin) {
      continue;
    }
    else {
      return_val &= ( digitalRead(otherPin) == HIGH );
    }
  }
  
  // verify sense pin is LOW
  return_val &= ( digitalRead(sense_pin) == LOW );
  
  // reset both pins
  set_highz(sense_pin);
  set_highz(toggle_pin);
  
  delay(2); // wait for pins to settle

  return return_val;
}

boolean led_on(int led_num) {
    if (led_num < 0 || led_num >= NUM_CHARLIEPLEXED_LEDS) {
      return false;
    }
    cpled_set(charlieplexed_leds[led_num], HIGH);
    return true;
}

boolean led_off(int led_num) {
    if (led_num < 0 || led_num >= NUM_CHARLIEPLEXED_LEDS) {
      return false;
    }
    cpled_set(charlieplexed_leds[led_num], LOW);
    return true;
}

void reset_pin(int pin_num) {
  pinMode(pin_num, INPUT);
  digitalWrite(pin_num, LOW);
}

void init_testpin_map(void) {
    testpin_map[0] = 18;            // D0 => A0
    testpin_map[1] = 19;            // D1 => A1
    testpin_map[2] = 20;            // D2 => A2
    testpin_map[3] = 21;            // D3 => A3
    testpin_map[4] = 22;            // D4 => A4
    testpin_map[5] = 23;            // D5 => A5
                                    //
    testpin_map[18] = 0;            // A0 => D0
    testpin_map[19] = 1;            // A1 => D1
    testpin_map[20] = 2;            // A2 => D2
    testpin_map[21] = 3;            // A3 => D3
    testpin_map[22] = 4;            // A4 => D4
    testpin_map[23] = 5;            // a5 => D5

    testpin_map[12] = 8;
    testpin_map[8] =  12;
    // left/right
    testpin_map[13] = 15;
    testpin_map[15] = 13;
    // space/click
    testpin_map[7] = 6;
    testpin_map[6] = 7;
    
}

boolean check_for_test_harness(void) {
  boolean should_test = true;
  for (int i=0; i < NUM_INPUTS; i++) {
    int toggle_pin = pinNumbers[i];
    // skip buttons for test detection.
    if (toggle_pin == 6 || toggle_pin == 7 || toggle_pin == 8 || toggle_pin == 12 || toggle_pin == 13 || toggle_pin == 15) {
      continue;
    }
    int sense_pin = testpin_map[toggle_pin];
    boolean result = test_pin(toggle_pin, sense_pin);
//    if (result) {
//           Keyboard.println("result: ("+String(toggle_pin)+","+String(sense_pin)+") == true");
//    }
//    else {
//           Keyboard.println("result: ("+String(toggle_pin)+","+String(sense_pin)+") == false");
//    }
    delay(25);
    should_test &= result;
  }
  return should_test;
}

boolean test_board(void) {
  boolean should_test = true;
  for (int i=0; i < NUM_INPUTS; i++) {
    int toggle_pin = pinNumbers[i];
    int sense_pin = testpin_map[toggle_pin];
    boolean result = test_pin(toggle_pin, sense_pin);
    if (!result) {
        Keyboard.println("testing input "+String(toggle_pin)+" ==> "+String(sense_pin)+"... FAILED");
    }
    delay(25);
    should_test &= result;
  }
  return should_test;
}

/*
#define CPLED_UP                 0
#define CPLED_DOWN               1
#define CPLED_LEFT               2
#define CPLED_RIGHT              3
#define CPLED_SPACE              4
#define CPLED_CLICK              5
*/

int update_finger_state(int test_input, int current_state) {
  if (inputs[test_input].pressed) {
    return current_state+1;
  }
  else {
    return current_state;
  }
}

boolean do_finger_test(void) {
  current_state = WAITING_FOR_UP;
  int ms_waited_for_fingers = FINGER_TEST_WAIT_TIME;
  
  while (ms_waited_for_fingers-- > 0) {
      updateMeasurementBuffers();
      updateBufferSums();
      updateBufferIndex();
      updateInputStates();
      sendMouseButtonEvents();
      sendMouseMovementEvents();
      cycleLEDs();
      updateOutLEDs();

      switch (current_state) {
        case WAITING_FOR_UP:
          cpled_set(charlieplexed_leds[CPLED_UP], HIGH);
          current_state = (state) update_finger_state(0, current_state); // UP
          break;
        case WAITING_FOR_DOWN:
          cpled_set(charlieplexed_leds[CPLED_DOWN], HIGH);
          current_state = (state) update_finger_state(1, current_state); // DOWN
          break;
        case WAITING_FOR_LEFT:
          cpled_set(charlieplexed_leds[CPLED_LEFT], HIGH);
          current_state = (state) update_finger_state(2, current_state); // LEFT
          break;       
        case WAITING_FOR_RIGHT:
          cpled_set(charlieplexed_leds[CPLED_RIGHT], HIGH);
          current_state = (state) update_finger_state(3, current_state); // RIGHT
          break;         
        case WAITING_FOR_SPACE:
          cpled_set(charlieplexed_leds[CPLED_SPACE], HIGH);
          current_state = (state) update_finger_state(4, current_state); // SPACE
          break;           
        case WAITING_FOR_CLICK:
          cpled_set(charlieplexed_leds[CPLED_CLICK], HIGH);
          current_state = (state) update_finger_state(5, current_state); // UP
          break;
        case MAX_STATES:
          return true;
      }
      delay(1);
  }
  return false;
}

void failure_waggle(void) {
  int waggle_time = 200;
  // WIGGLE
  for(int i=0; i<20; i++)
  {
    // SPACE
    cpled_set(charlieplexed_leds[CPLED_SPACE], HIGH);
    delay(waggle_time);    

    // CLICK
    cpled_set(charlieplexed_leds[CPLED_CLICK], HIGH);
    delay(waggle_time);    
  }
  
  set_highz(inputLED_a);
  set_highz(inputLED_b);
  set_highz(inputLED_c);
}

void success_back_leds(void) {
  for(int i=0; i<3; i++) {
    digitalWrite(outputK, HIGH);
    TXLED1;
    digitalWrite(outputM, HIGH);
    RXLED1;
    delay(200);
    
    digitalWrite(outputK, LOW);
    TXLED0;
    digitalWrite(outputM, LOW);
    RXLED0;
    delay(200);
  }  
}

void do_debug(void) {
  /* 
   * is the test-harness hooked up?
   * if so, do a test of all input pins
   * and "type" the test result
   */
   init_testpin_map();
   delay(DEBUG_WAIT_TIME_MS);
   if (!check_for_test_harness()) {
     return;
   }
   else {
     success_back_leds();
     boolean finger_test_result = do_finger_test();
     if (finger_test_result) {
       Keyboard.println("MaKey MaKey self-test result: PASSED :-)");
     } else {
       Keyboard.println("MaKey MaKey self-test result: FAILED!");
       failure_waggle();
     }
   }
}

 


