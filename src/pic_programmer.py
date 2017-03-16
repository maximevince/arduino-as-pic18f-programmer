#!/usr/bin/python

"""
Copyright (C) 2012  kirill Kulakov & Jose Carlos Granja

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

from serial import *
import getopt
import sys
from Hex import *

mcus = (["18f2455", 0x1260], ["18f2550", 0x1240], ["18f4455", 0x1202], ["18f4550", 0x1200], ["18f2420", 0x1140],
        ["18f2520", 0x1100], ["18f4420", 0x10C0], ["18f4520", 0x1080])


def getOut():
    print "For help use --help"
    sys.exit(2)


def main():
    try:
        options, arguments = getopt.getopt(sys.argv[1:], 'hp:aP:i:le', ['help', 'port=', 'list', 'erase'])
    except getopt.error, msg:
        print msg
        getOut()

    MCU = ""
    PORT = ""
    FILENAME = ""
    ERASE_MODE = False

    SLEEP_TIME = 0.00001

    for opt, arg in options:
        if opt in ('-h', '--help'):
            helpFile = open("help", "r")
            print helpFile.read() + "\n"
            sys.exit(0)
        elif opt in ('-l', '--list'):
            print "Supported MCUs:"
            for mcu, i in mcus:
                print "pic" + mcu
            sys.exit(0)
        elif opt in ('-e', '--erase'):
            ERASE_MODE = True
        elif opt in ('-p'):
            MCU = arg
        elif opt in ('-P', '--port'):
            PORT = arg
        elif opt in ('-i'):
            FILENAME = arg

    if PORT == "":
        PORT = '/dev/ttyACM0'
    if FILENAME == "" and not ERASE_MODE:
        print "You need to select an hex file with -i option"
        getOut()

    print ("Connecting to arduino..."),

    # Open Serial port
    try:
        arduino = Serial(PORT, 9600)
    except SerialException, msg:
        print msg
        sys.exit(2)

    time.sleep(2)

    # Say hello to Arduino
    arduino.write('HX' + '\r\n')
    if arduino.read() == 'H':
        # If Arduino responds, check for the mcu
        print ("\tSuccess")
        print ("Connecting to the mcu..."),
        arduino.flushInput()
        arduino.write('DX' + '\r\n')  # asking Arduino for the DeviceID
        time.sleep(SLEEP_TIME)
        deviceID = ord(arduino.read()) + ord(arduino.read()) * 256

        # checking the MCU
        mcu_found = False
        for mcu, ID in mcus:
            if ID == deviceID:
                print "\tYour MCU: " + mcu
                mcu_found = True

        if mcu_found:
            print "Erasing chip............",
            arduino.flushInput()
            arduino.write('EX' + '\r\n')
            if arduino.read() == "K":
                print "\tSuccess"

                if not ERASE_MODE:
                    # open and parse the hex file
                    hexFile = Hex(FILENAME)

                    print "Programming the fuse bits...",
                    for i in range(0x0F):
                        if hexFile.fuseChanged(i):
                            buf = "C" + hex(i)[2:] + hex(hexFile.getFuse(i))[2:] + "X"
                            arduino.write(buf.upper())
                            time.sleep(SLEEP_TIME)
                    print "\tSuccess"
                    print "Programming flash memory...",

                    address = 0
                    while address < 0x8000:
                        for i in range(0x20):
                            if hexFile.getData(address + i) != 0xFF:
                                buf = "W"
                                buf += str(hex(address)[2:].zfill(4))
                                for j in range(0x20):
                                    buf += str(hex(hexFile.getData(address + j))[2:].zfill(2))
                                buf += "X"
                                arduino.write(buf.upper())
                                time.sleep(SLEEP_TIME)
                                break
                        address += 0x20

                    print "\tSuccess"
            else:
                print "Couldn't erase the chip."
                sys.exit(1)
        else:
            print "MCU not recognized. Check the list of compatible MCU'S and/or check your wire conections."
            sys.exit(1)
    else:
        print "Couldn't connect to the Arduino."
        sys.exit(2)
    arduino.close()


if __name__ == "__main__":
    main()
