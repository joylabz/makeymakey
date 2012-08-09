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
     boolean result = test_board();
     if (result) {
       Keyboard.println("MaKey MaKey self-test result: PASSED :-)");
     } else {
       Keyboard.println("MaKey MaKey self-test result: FAILED!");
     }
   }

}

