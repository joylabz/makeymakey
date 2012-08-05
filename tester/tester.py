import os
import time
import base64
import string
import random
import optparse

import serial

def validate_keys(serial_port):
    pass

def random_str(length=5):
    return base64.urlsafe_b64encode(os.urandom(length))[:length]

def shellmode(serial_port):
    print "entering shell mode..."
    print ">>>",
    while 1:
        msg = raw_input()
        try:
            serial_port.write(msg+'\n')
            print serial_port.readline().strip()
        except serial.serialutil.SerialException:
            pass
        if (msg == "EXIT"):
            return
        print ">>>",

def test_makeymakey(serial_port):
    # 
    pass

def main():
    """
        send a series of commands to the MakeyMakey board and check for the correct response
    """
    parser = optparse.OptionParser(usage="usage: %prog -s <serial_device>")
    parser.add_option("-s", "--serial_dev",
                      metavar="serial_dev", help="full path to serial device", default='/dev/tty.usbmodemfa131')
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
                    shellmode(serial_port)
            except (serial.serialutil.SerialException, OSError):
                pass
            time.sleep(.1)

if __name__ == "__main__":
    main()