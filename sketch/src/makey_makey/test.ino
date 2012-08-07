#include "test.h"

void debug_start(void) {
  Serial.begin(9600);
  Serial.setTimeout(100);
}

void debug_end(void) {
  Serial.end();
}

boolean test_pin(int toggle_pin, int sense_pin) {
  boolean return_val = true;
  
  // reset both pins
  set_highz(sense_pin);
  set_highz(toggle_pin);
  
  // put sense_pin in input mode
  // no internal pullup because we have externals on the board
  pinMode(sense_pin, INPUT);
  digitalWrite(sense_pin, LOW);
  
  // verify sense pin is HIGH
  return_val &= ( digitalRead(sense_pin) == HIGH );
  
  // pull toggle pin low
  set_gnd(toggle_pin);
  
  // wait for it to fall
  delay(2);
  
  // verify that all other inputs are STILL HIGH!
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

//void reset_all_pins() {
//  // turn off all LEDs
//  for (int i = 0; i < NUM_LEDS; i++) {
//    reset_pin(leds[i]);
//  }
//  // make all inputs, well, inputs :)
//  for (int i = 0; i < NUM_INPUTS; i++) {
//    reset_pin(inputs[i]);
//  }
//}

int parse_int_string(String int_string) {
    char int_buf[int_string.length()+1];
    int_string.toCharArray(int_buf, sizeof(int_buf));
    return atoi(int_buf);
}

boolean handle_command(String cmd) {
  /* 
   * Parse and execute commands. Return false when it's
   * time to exit debug mode.
   * TODO: rewrite this to use char* instead of string you lazybones
   */
  if (cmd.startsWith("LEDON:")) {
    String led_num_substr = cmd.substring(cmd.indexOf(":")+1);
    int led_num = parse_int_string(led_num_substr);
    boolean led_status = led_on(led_num);
    if (led_status) {
      Serial.println(cmd+" ==> success");
    }
    else {
      Serial.println(cmd+" ==> failure");
    }
  }
  else if (cmd.startsWith("LEDOFF:")) {
    String led_num_substr = cmd.substring(cmd.indexOf(":")+1);
    int led_num = parse_int_string(led_num_substr);
    boolean led_status = led_off(led_num);
    if (led_status) {
      Serial.println(cmd+" ==> success");
    }
    else {
      Serial.println(cmd+" ==> failure");
    }
  }
  else if (cmd.startsWith("TESTPIN:")) {
    String both_pins_substr = cmd.substring(cmd.indexOf(":")+1);
    int toggle_pin = parse_int_string(both_pins_substr.substring(0, both_pins_substr.indexOf(",")));
    int sense_pin = parse_int_string(both_pins_substr.substring(both_pins_substr.indexOf(",")+1));
    boolean testResult = test_pin(toggle_pin, sense_pin);
    if (testResult) {
      Serial.println(cmd+" ==> success");
    }
    else {
      Serial.println(cmd+" ==> failure");
    }
  }
  else if (cmd.startsWith("DANCE")) {
    danceLeds();
    Serial.println(cmd+" ==> success");
  }
  else if (cmd.equals("EXIT")) {
    Serial.println(cmd+" ==> success");
    return false;
  }
  return true;
}

String get_command(void) {
  /* 
   * Read (until '\n') from serial and return the full command in a 
   * String object
   */
  static char cmd_buffer[MAX_CMD_LENGTH];
  memset(cmd_buffer, '\0', MAX_CMD_LENGTH);
  int bytesRead = Serial.readBytesUntil('\n', cmd_buffer, MAX_CMD_LENGTH);
  return String(cmd_buffer);
}

void listen_for_debug(void) {
  /* 
   * enable serial input and listen for a "DEBUG" command
   * if none is recieved after DEBUG_WAIT_TIME_MS boot the sketch
   * else jump to debug mode
   */
  int ms_waited = 0;
  boolean in_debug_mode = false;
  while (in_debug_mode || (ms_waited < DEBUG_WAIT_TIME_MS)) {
    if (Serial.available() > 0) {
      String cmd = get_command();
      if (cmd == "DEBUG") {
        in_debug_mode = true;
        ms_waited = DEBUG_WAIT_TIME_MS+1; // don't let us hit this again.
        Serial.println("DEBUGOK");
      }
      else {
       in_debug_mode = handle_command(cmd);
      }
    }
    delay(1);
    ms_waited += 1;
  }
}
