import os
import re
import time
import base64
import string
import random
import optparse

import serial

pin_status_matcher = re.compile(r'TESTPIN:(?P<toggle_pin>\d+?),(?P<sense_pin>\d+?) ==> (?P<outcome>success|failure)')

# a 1-many mapping of pin numbers that are connected
pin_connection_map = {
    0 : [18],   # D0 => A0
    1 : [19],   # D1 => A1
    2 : [20],   # D2 => A2
    3 : [21],   # D3 => A3
    4 : [22],   # D4 => A4
    5 : [23],   # D5 => A5

    18 : [0],   # A0 => D0
    19 : [1],   # A1 => D1
    20 : [2],   # A2 => D2
    21 : [3],   # A3 => D3
    22 : [4],   # A4 => D4
    23 : [5],   # a5 => D5

}

"""
#define CPLED_UP                 0
#define CPLED_DOWN               1
#define CPLED_LEFT               2
#define CPLED_RIGHT              3
#define CPLED_SPACE              4
#define CPLED_CLICK              5
"""
charlieplexed_leds = [
    0,
    1,
    2,
    3,
    4,
    5,
]

def random_str(length=5):
    return base64.urlsafe_b64encode(os.urandom(length))[:length]

def shellmode(serial_port):
    print "entering shell mode..."
    print "(MM shell) >>>",
    while 1:
        msg = raw_input()
        try:
            serial_port.write(msg+'\n')
            print serial_port.readline().strip()
        except serial.serialutil.SerialException:
            pass
        if (msg == "EXIT"):
            return
        print "(MM shell) >>>",

def parse_pintest_response(response_str):
    # TESTPIN:18,0 ==> success
    response_str = response_str.strip()
    response_match = pin_status_matcher.match(response_str)
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
    pass

def test_leds(serial_port):
    for cpled in charlieplexed_leds:
        pass

def test_backside_km_pins(serial_port):
    failures = []
    test_message = "TESTPIN:%s,%s\n"
    for toggle_pin, sense_pins in pin_connection_map.iteritems():
        for sense_pin in sense_pins:
            msg = test_message % (toggle_pin, sense_pin)
            serial_port.write(msg)
            print "testing: ",(toggle_pin, sense_pin),"...",
            response = serial_port.readline().strip()
            print response
            pins, succeeded = parse_pintest_response(response)
            if not succeeded:
                failures.append(pins)
    if failures:
        print "The following pins FAILED:"
        for pin_pair in failures:
            print "\t(%s,%s)" % pin_pair

def main():
    """
        send a series of commands to the MakeyMakey board and check for the correct response
    """
    parser = optparse.OptionParser(usage="usage: %prog -s <serial_device>")
    parser.add_option("-s", "--serial_dev",
                      metavar="serial_dev", help="full path to serial device", default='/dev/tty.usbmodemfd121')
    (options, args) = parser.parse_args()
    if not options.serial_dev:
        parser.error("you must specify a serial port!")

    # loop, looking for a device on our port
    # when we see one, open the port and put the device in debug mode
    while 1:
        if not os.path.exists(options.serial_dev):
            continue
        else:
            try:
                serial_port = serial.Serial(options.serial_dev, 9600, timeout=5)
                # print "device exists!"
                serial_port.write("DEBUG")
                if (serial_port.readline().strip() == "IN DEBUG MODE!"):
                    print "Put MM in debug mode"
                    test_backside_km_pins(serial_port)
                    shellmode(serial_port)
            except (serial.serialutil.SerialException, OSError):
                pass
            time.sleep(.1)

if __name__ == "__main__":
    main()