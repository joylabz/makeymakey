#ifndef TEST_H
#define TEST_H

/* 
 * how long (in ms) the board will sleep on startup (waiting for computer to see HID)
 * before checking for a test harness and then starting the sketch 
 */
#define DEBUG_WAIT_TIME_MS 500 
#define FINGER_TEST_WAIT_TIME 3000
#define FINGER_TEST_TOUCH_BONUS_MS 2000
#define SUCCESS_BLINK_COUNT 3
#define FAILURE_BLINK_COUNT 6
#define BLINK_DURATION_MS 400
#define EEPROM_TESTRESULT_ADDRESS 256

void updateInputStatesNoPress(void);
boolean test_pin(int toggle_pin, int sense_pin);
boolean led_on(int led_num);
boolean led_off(int led_num);
void reset_pin(int pin_num);
void init_testpin_map(void);
boolean check_for_test_harness(void);
boolean test_board(void);
int update_finger_state(int test_input, int current_state);
boolean do_finger_test(void);
void failure_waggle(void);
void success_waggle(void);
void success_back_leds(void);
void do_debug(void);

enum state { 
   WAITING_FOR_UP = 0, 
   WAITING_FOR_DOWN, 
   WAITING_FOR_LEFT, 
   WAITING_FOR_RIGHT, 
   WAITING_FOR_SPACE, 
   WAITING_FOR_CLICK,
   MAX_STATES,
} current_state;

#endif

