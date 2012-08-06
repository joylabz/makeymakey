#ifndef TEST_H
#define TEST_H

#define DEBUG_WAIT_TIME_MS 1000 /* how long (in ms) the board will listen on bootup for a DEBUG signal before starting the sketch */
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

#endif TEST_H

