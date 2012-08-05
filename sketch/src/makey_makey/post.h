#include "Arduino.h"

#define NUM_INPUTS 18
#define NUM_LEDS 2
#define DEBUG_WAIT_TIME_MS 5000
#define MAX_CMD_LENGTH 15


void debug_start(void);
void debug_end(void);
boolean test_pin(int toggle_pin, int sense_pin);
boolean led_on(int led_num);
boolean led_off(int led_num);
void reset_pin(int pin_num);
int parse_int_string(String int_string);
boolean handle_command(String cmd);
String get_command(void);
void listen_for_debug(void);

