
void debug_start(void) {
  Serial.begin(9600);
  Serial.setTimeout(100);
}

void debug_end(void) {
  Serial.end();
}

boolean test_pin(int toggle_pin, int sense_pin) {
  boolean return_val = false;
  
  // reset both pins
  pinMode(sense_pin, INPUT);
  digitalWrite(sense_pin, LOW);
  pinMode(toggle_pin, INPUT);
  digitalWrite(toggle_pin, LOW);
  
  // put sense_pin in input mode
  // no internal pullup because we have externals on the board
  pinMode(sense_pin, INPUT);
  digitalWrite(sense_pin, LOW);
  
  // verify sense pin is HIGH
  return_val &= ( digitalRead(sense_pin) == true );
  
  // pull toggle pin low
  pinMode(toggle_pin, OUTPUT);
  digitalWrite(toggle_pin, LOW);
  
  // verify sense pin is LOW
  return_val &= ( digitalRead(sense_pin) == false );
  
  // reset both pins
  pinMode(sense_pin, INPUT);
  digitalWrite(sense_pin, LOW);
  pinMode(toggle_pin, INPUT);
  digitalWrite(toggle_pin, LOW);
  
  return return_val;
}

boolean led_on(int led_num) {
  // TODO: assert led_num
  pinMode(led_num, OUTPUT);
  digitalWrite(led_num, HIGH);
}

boolean led_off(int led_num) {
  // TODO: assert led_num
  pinMode(led_num, INPUT);
  digitalWrite(led_num, LOW);
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
   */
  if (cmd.startsWith("LEDON:")) {
    String led_num_substr = cmd.substring(cmd.indexOf(":")+1);
    int led_num = parse_int_string(led_num_substr);
    led_on(led_num);
    Serial.println("LEDON: "+String(led_num));
  }
  else if (cmd.startsWith("LEDOFF:")) {
    String led_num_substr = cmd.substring(cmd.indexOf(":")+1);
    int led_num = parse_int_string(led_num_substr);
    led_off(led_num);
    Serial.println("LEDOFF: "+String(led_num));
  }
  else if (cmd.startsWith("TESTPIN:")) {
    String both_pins_substr = cmd.substring(cmd.indexOf(":")+1);
    int toggle_pin = parse_int_string(both_pins_substr.substring(0, cmd.indexOf(",")));
    int sense_pin = parse_int_string(both_pins_substr.substring(cmd.indexOf(",")+1));
    boolean testResult = test_pin(toggle_pin, sense_pin);
    Serial.println(cmd+"substring:"+both_pins_substr+"==>"+testResult);
  }
  else if (cmd.startsWith("DANCE")) {
    Serial.println("dancing leds...");
    danceLeds();
  }
  else if (cmd.equals("EXIT")) {
    Serial.println("MakeyMakey exiting debug mode...");
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
   * enable serial input and listen for a "POST" command
   * if none is recieved after POST_WAIT_TIME_MS boot the sketch
   * else jump to debug mode
   */
  int ms_waited = 0;
  boolean in_debug_mode = false;
  while (in_debug_mode || (ms_waited < DEBUG_WAIT_TIME_MS)) {
    if (Serial.available() > 0) {
      String cmd = get_command();
      if (cmd == "DEBUG") {
        in_debug_mode = true;
        ms_waited = DEBUG_WAIT_TIME_MS+1; // don't want to hit this again.
        Serial.println("IN DEBUG MODE!");
      }
      else {
       in_debug_mode = handle_command(cmd);
      }
    }
    delay(1);
    ms_waited += 1;
  }
}
