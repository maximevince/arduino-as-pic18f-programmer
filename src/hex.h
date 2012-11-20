/*
Copyright (C) 2012  kirill Kulakov

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

*/

typedef struct _hex *HEX;
typedef unsigned char data;


//constructor
HEX readHex(char *location);

//get data
data getData(HEX thehex,unsigned int address);

int fuseChanged(HEX thehex,data fuseID);

data getfuse(HEX thehex,data fuseID);