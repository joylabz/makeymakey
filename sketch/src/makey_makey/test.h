#ifndef TEST_H
#define TEST_H

/* 
 * how long (in ms) the board will sleep on startup (waiting for computer to see HID)
 * before checking for a test harness and then starting the sketch 
 */
#define DEBUG_WAIT_TIME_MS 500 

boolean test_pin(int toggle_pin, int sense_pin);
boolean led_on(int led_num);
boolean led_off(int led_num);
void reset_pin(int pin_num);
boolean test_board(void);
boolean check_for_test_harness(void);
void do_debug(void);

#endif TEST_H
