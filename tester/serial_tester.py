#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import time
import base64
import string
import random
import logging
import optparse

import serial

logger = logging.getLogger(__name__)
pintest_response_matcher = re.compile(r'TESTPIN:(?P<toggle_pin>\d+?),(?P<sense_pin>\d+?) ==> (?P<outcome>success|failure)')
ledtest_response_matcher = re.compile(r'LED(ON|OFF):(?P<led_num>\d+?) ==> (?P<outcome>success|failure)')
danceleds_response_matcher = re.compile(r'DANCE ==> (?P<outcome>success|failure)')
exit_response_matcher = re.compile(r'EXIT ==> (?P<outcome>success|failure)')

"""
int pinNumbers[NUM_INPUTS] = {
  12, 8, 13, 15, 7, 6,     // top of makey makey board
  5, 4, 3, 2, 1, 0,        // left side of female header, KEBYBOARD
  23, 22, 21, 20, 19, 18   // right side of female header, MOUSE
};

# pin_connection_name_map is a 1-many mapping of pin numbers that are connected
"""
pin_connection_name_map = {
    # D* pins (WASDFG) pulled down, A* pins (UP, DOWN, LEFT, RIGHT, LMOUSE, RIGHTMOUSE) tested
    0 : ([18],"G"),             # D0 => A0
    1 : ([19],"F"),             # D1 => A1
    2 : ([20],"D"),             # D2 => A2
    3 : ([21],"S"),             # D3 => A3
    4 : ([22],"A"),             # D4 => A4
    5 : ([23],"W"),             # D5 => A5
    # A* pins (UP, DOWN, LEFT, RIGHT, LMOUSE, RIGHTMOUSE) pulled down, D* pins (WASDFG) tested
    18 : ([0],"UP"),            # A0 => D0
    19 : ([1],"DOWN"),          # A1 => D1
    20 : ([2],"LEFT"),          # A2 => D2
    21 : ([3],"RIGHT"),         # A3 => D3
    22 : ([4],"LEFT_MOUSE"),    # A4 => D4
    23 : ([5],"RIGHT_MOUSE"),   # a5 => D5
    # up/down
    12 : ([8],"UP"),
    8 :  ([12],"DOWN"),
    # left/right
    13 : ([15],"LEFT"),
    15 : ([13],"RIGHT"),
    # space/click
    7 : ([6],"SPACE"),
    6 : ([7],"CLICK"),
}

"""
#define CPLED_UP                 0
#define CPLED_DOWN               1
#define CPLED_LEFT               2
#define CPLED_RIGHT              3
#define CPLED_SPACE              4
#define CPLED_CLICK              5
# a list of the "virtual pin numbers" for the charlieplexed LEDs
"""
charlieplexed_leds = {
    "UP"      :0,
    "DOWN"    :1,
    "LEFT "   :2,
    "RIGHT"   :3,
    "SPACE"   :4,
    "CLICK"   :5,
}

def random_str(length=5):
    return base64.urlsafe_b64encode(os.urandom(length))[:length]

def shellmode(serial_port):
    prompt = "(MM shell) >>>"
    print "entering shell mode..."
    print prompt,
    while 1:
        msg = raw_input()
        try:
            serial_port.write(msg+'\n')
            print serial_port.readline().strip()
        except serial.serialutil.SerialException:
            pass
        if (msg == "EXIT"):
            return
        print prompt,

def parse_pintest_response(response_str):
    # TESTPIN:18,0 ==> success
    response_str = response_str.strip()
    response_match = pintest_response_matcher.match(response_str)
    if not response_match:
        return None, False
    match_dict = response_match.groupdict()
    pins = (int(match_dict['toggle_pin']), int(match_dict['sense_pin']))
    if match_dict['outcome'] == "success":
        succeeded = True
    elif match_dict['outcome'] == "failure":
        succeeded = False
    return pins, succeeded

def parse_ledtest_response(response_str):
    # LEDON:1 ==> success
    response_str = response_str.strip()
    response_match = ledtest_response_matcher.match(response_str)
    if not response_match:
        print "Unknown LED response: '%s'" % response_str
        return None, False
    match_dict = response_match.groupdict()
    pin = int(match_dict['led_num'])
    if match_dict['outcome'] == "success":
        succeeded = True
    elif match_dict['outcome'] == "failure":
        succeeded = False
    return pin, succeeded

def parse_danceleds_response(response_str):
    response_str = response_str.strip()
    response_match = danceleds_response_matcher.match(response_str)
    if not response_match:
        print "Unknown DANCE response: '%s'" % response_str
        return False
    else:
        return True

def parse_exit_response(response_str):
    response_str = response_str.strip()
    response_match = exit_response_matcher.match(response_str)
    if not response_match:
        print "Unknown EXIT response: '%s'" % response_str
        return False
    else:
        return True

def build_testled_msg(lednum, enabled):
    if enabled:
        return "LEDON:%i\n" % lednum
    else:
        return "LEDOFF:%i\n" % lednum

def build_testpin_msg(toggle_pin, sense_pin):
    return "TESTPIN:%i,%i\n" % (toggle_pin, sense_pin)

def build_danceleds_msg():
    return "DANCE"

def build_exit_msg():
    return "EXIT"

def dance_leds(serial_port):
    msg = build_danceleds_msg()
    print "dancing LEDs....",
    serial_port.write(msg)
    time.sleep(2)
    succeeded = parse_danceleds_response(serial_port.readline().strip())
    print "OK" if succeeded else "FAILED"
    return succeeded

def exit_test_mode(serial_port):
    msg = build_exit_msg()
    print "exiting test mode...",
    serial_port.write(msg)
    succeeded = parse_exit_response(serial_port.readline().strip())
    print "OK" if succeeded else "FAILED"
    return succeeded

def test_input_pins(serial_port):
    failed_pinpairs = []
    for toggle_pin, (sense_pins,toggle_pin_name) in pin_connection_name_map.iteritems():
        for sense_pin in sense_pins:
            msg = build_testpin_msg(toggle_pin, sense_pin)
            serial_port.write(msg)
            print "testing input %3i ==> %3i....." % (toggle_pin, sense_pin),
            pins, succeeded = parse_pintest_response(serial_port.readline().strip())
            print "OK" if succeeded else "FAILED"
            if not succeeded and pins:
                failed_pinpairs.append((pins,toggle_pin_name))
    if failed_pinpairs:
        return failed_pinpairs
    else:
        return None

def test_leds(serial_port):
    failed_leds = []
    for ledname, cpled in charlieplexed_leds.iteritems():
        print "testing led %5s (%i)........." % (ledname, cpled),
        msg = build_testled_msg(cpled, 1)
        serial_port.write(msg)
        pin, succeeded = parse_ledtest_response(serial_port.readline().strip())
        msg = build_testled_msg(cpled, 0)
        serial_port.write(msg)
        pin2, succeeded2 = parse_ledtest_response(serial_port.readline().strip())
        print "OK" if (succeeded and succeeded2) else "FAILED"
        if not (succeeded and succeeded2):
            failed_leds.append((pin, ledname))
    return failed_leds
                
def test_mm(serial_port):
    failed_pinpairs = test_input_pins(serial_port)
    failed_leds = test_leds(serial_port)
    if failed_pinpairs:
        print "The following pins FAILED:"
        for pinpair,name in failed_pinpairs:
            print "\t%3i ==> %3i (%s)" % (pinpair[0],pinpair[1],name)
            
    if failed_leds:
        print "The following LEDs FAILED:"
        for pin,ledname in failed_leds:
            print "\t%i (%s)" % (pin, ledname)
    
    return (failed_pinpairs or failed_leds)

def disconnect_mm(serial_port):
    exit_success = exit_test_mode(serial_port)
    if not exit_success:
        print "Exiting test mode FAILED"
    serial_port.close()

# WAIT FOR KEYS
# def ask_for_key():
# def wait_for_keys():
#     print "press UP"
    
def enter_debug_mode(serial_port):
    serial_port.write("DEBUG")
    response = serial_port.readline().strip()
    if (response == "DEBUGOK"):
        print "Put MM in debug mode!"
        return True
    else:
        return False

def wait_for_disconnect(serial_dev):
    print "Please disconnect MM."
    while os.path.exists(serial_dev):
        time.sleep(.1)
    print "MM disconnected OK!\n\n"

def main():
    """
        send a series of commands to the MakeyMakey board and check for the correct response
    """
    parser = optparse.OptionParser(usage="usage: %prog -s <serial_device>")
    parser.add_option("-s", "--serial_dev",
                      metavar="serial_dev", help="full path to serial device", default='/dev/tty.usbmodemfd121')
    parser.add_option("-e", "--shell_mode",
                    metavar="shell_mode", help="dump to shell mode after testing", default=False, action="store_true")

    (options, _) = parser.parse_args()
    if not options.serial_dev:
        parser.error("you must specify a serial port!")

    # loop, looking for a device on our port
    # when we see one, open the port and put the device in debug mode
    print "waiting for MakeyMakey on %s" % options.serial_dev
    while 1:
        if not os.path.exists(options.serial_dev):
            continue
        else:
            try:
                serial_port = serial.Serial(options.serial_dev, 9600, timeout=.2)
                if enter_debug_mode(serial_port):
                    test_mm(serial_port)
                    if options.shell_mode:
                        shellmode(serial_port)
                    else:
                        disconnect_mm(serial_port)
                    wait_for_disconnect(options.serial_dev)
                else:
                    continue
            except (serial.serialutil.SerialException, OSError):
                pass
            time.sleep(.1)

if __name__ == "__main__":
    main()