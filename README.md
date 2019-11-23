# Arduino as PIC18F programmer

Version 0.4

* 2019 - Added support for PIC18F2xK22/PIC18F4xK22, cleaned up and more by Maxime Vincent - https://github.com/maximevince/arduino-as-pic18f-programmer
* 2017 - Improved by Xerxes Rånby - https://bitbucket.org/xranby/arduino-as-pic18f-programmer-for-linux
* 2017 - Adapted for Linux by Jose Carlos Granja - https://bitbucket.org/JoseFuzzNo/arduino-as-pic18f-programmer-for-linux
* Copyright (C) 2012  kirill Kulakov - https://sites.google.com/site/thehighspark/home

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

----------------------------------------------------------------

### INSTALLATION:

- Debian/Ubuntu: 
```
apt install arduino python-serial
```

### HOW TO:

1. Upload the sketch to the arduino (Adjust TOGGLE_MCLR define to your needs)
2. Connect the chip to the arduino (PGM is not used for PIC18F2xK22/PIC18F4xK22 series)
3. Run `./pic_programmer.py -i HEX_FILE`

* For help run `./pic_programmer.py -h`

Thats it!

----------------------------------------------------------------

### SUPPORTED MCUs:

 * PIC18F2455
 * PIC18F2550
 * PIC18F4455
 * PIC18F4550
 * PIC18F2420
 * PIC18F2520
 * PIC18F4420
 * PIC18F4520
 * PIC18F45K22,  PIC18LF45K22
 * PIC18F25K22,  PIC18LF25K22
 * PIC18F23K22,  PIC18LF23K22
 * PIC18F24K22,  PIC18LF24K22
 * PIC18F26K22,  PIC18LF26K22
 * PIC18F43K22,  PIC18LF43K22
 * PIC18F44K22,  PIC18LF44K22
 * PIC18F46K22,  PIC18LF46K22

----------------------------------------------------------------

### RELEASE NOTES:

version 0.4
 - Support for PIC18F2xK22/PIC18F4xK22 series (Only PIC18F26K22 was tested)
 - Code clean-up

version 0.3
 - Support loading EEPROM, Id and comment sections from HEX file.
 - Programming is faster
   Arduino UNO serial USB operate at 2000000 baud instead of 9600 baud
 - Verification that Program, ID & EEPROM memory have been programmed correctly

version 0.2
 - has a new PC-side software written in Python. It's simpler than the old one and a bit faster, it uses the pySerial library and allows to program the following microcontrollers:
 * 	18F2420
 * 	18F2455
 * 	18F2520
 * 	18F2550 (* tested)
 * 	18F4420
 * 	18F4455
 * 	18F4520
 * 	18F4550 (* tested)

