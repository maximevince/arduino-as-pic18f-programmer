Version 0.1


Copyright (C) 2012  kirill Kulakov - https://sites.google.com/site/thehighspark/home

Adapted for Linux by Jose Carlos Granja - https://bitbucket.org/JoseFuzzNo/arduino-as-pic18f-programmer-for-linux

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


thanks to Teunis van Beelen for the 'rs232' lib, you could find him over him website teuniz.net


This linux version uses the implementation of Borland's conio.h by Piotr Helka (piotr.helka@nd.e-wro.pl) made arround ncurses library, so you have to install it first. Try "sudo apt-get install libncurses5*" in debian distributions.

----------------------------------------------------------------

HOW TO:

1. Compile with make (deps: ncurses)
2. Upload the sketch to the arduino
3. Connect the chip (pic18f2550) to the arduino
4. Run ./pic_programmer HEX_FILE

Thats it!

----------------------------------------------------------------

TODO:

Support for all PIC18F.
