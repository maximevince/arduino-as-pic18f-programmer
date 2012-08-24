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

#include "HEX.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct _configbit{
	data value;
	unsigned int changed;
} configbit;

typedef struct _hex{
	data memory[0x8000];
	FILE *filePtr;
	unsigned int offset;
	configbit fuse[0xf];
};


static int hexCharToInt(char cData){
	switch(cData){
		case '0': return 0; break;
		case '1': return 1; break;
		case '2': return 2; break;
		case '3': return 3; break;
		case '4': return 4; break;
	    case '5': return 5; break;
		case '6': return 6; break;
		case '7': return 7; break;
		case '8': return 8; break;
		case '9': return 9; break;
		case 'A': return 10; break;
		case 'B': return 11; break;
		case 'C': return 12; break;
		case 'D': return 13; break;
		case 'E': return 14; break;
		case 'F': return 15; break;
		default: return -1;
	}
}

static int hexStringToInt(char *sData,unsigned int length){
	unsigned int i;
	int iResult=0;

	for(i=0;i<length;i++){
		if( ( iResult += hexCharToInt(*sData) ) == -1 ) return -1;
		iResult *= 0x10;
		sData++;
	}

	iResult /= 0x10;

	return iResult;
}

static int reformat(HEX thehex,char *buffer){
	
	unsigned int iSize,iAddress,iRecord,i;
	data temp;

	if( ( iSize = hexStringToInt(buffer,2) ) == -1 ) return 1;

	buffer += 2;

	if( ( iAddress = hexStringToInt(buffer,4) ) == -1 ) return 1;

	buffer += 4;

	//record type
	if( ( iRecord = hexStringToInt(buffer,2) ) == -1 ) return 1;

	if( iRecord == 4 ){

		buffer += 2;

		if( ( temp = hexStringToInt(buffer,2) ) == -1 ) return 1;

		thehex->offset += temp<<24;

		buffer += 2;

		if( ( temp = hexStringToInt(buffer,2) ) == -1 ) return 1;

		thehex->offset = temp<<16;

	} else if ( iRecord == 2 ) {

		buffer += 2;

		if( ( temp = hexStringToInt(buffer,2) ) == -1 ) return 1;
		
		thehex->offset += temp<<12;

		buffer += 2;
		 
		if( ( temp = hexStringToInt(buffer,2) ) == -1 ) return 1;
		
		thehex->offset = temp<<4;

	} else if ( iRecord == 0 ) {

		if( iAddress + (thehex->offset) < 0x8000 ){
			for(i=0;i<(iSize/2);i++){
				buffer += 2;
				if( ( thehex->memory[iAddress + i*2 + (thehex->offset)  ] = hexStringToInt(buffer,2) ) == -1 )
					return 1; 
				buffer += 2;
				if( ( thehex->memory[iAddress + i*2 + (thehex->offset) + 1 ] = hexStringToInt(buffer,2) ) == -1 )
					return 1;
			}
		} else if ( iAddress + (thehex->offset) >= 0x300000 && iAddress + (thehex->offset) <= 0x30000f){
			for(i=0;i<iSize;i++){
				buffer += 2;
				if((thehex->fuse[ iAddress + (thehex->offset) + i - 0x300000 ].value = hexStringToInt(buffer,2)) == -1) 
					return 1;
				thehex->fuse[ iAddress + (thehex->offset) + i - 0x300000 ].changed = 1;
			}
		} else return 1;

	}
	

	return 0;
}



HEX readHex(char *location){

	HEX thehex;
	char buffer[128];
	unsigned int i;
	

	if( ( thehex = (HEX)malloc(sizeof(struct _hex)) ) == NULL )
		return NULL;

	if( ( thehex->filePtr = fopen(location,"r") ) == NULL )
		return NULL;

	for(i=0;i<0x8000;i++)
		thehex->memory[i] = 0xFF;

	for(i=0;i<0xf;i++)
		thehex->fuse[i].changed = 0;

	thehex->offset = 0;

	while( fscanf( thehex->filePtr,":%128[^\n]%*c",buffer) != EOF ){
		if( reformat(thehex,buffer) == 1 )
			return NULL;
	}
	
	fclose(thehex->filePtr);

	return thehex;
}

//get data
data getData(HEX thehex,unsigned int address){
	return thehex->memory[address];
}

int fuseChanged( HEX thehex,data fuseID ){
	return thehex->fuse[fuseID].changed;
}

data getfuse(HEX thehex,data fuseID){
	return thehex->fuse[fuseID].value;
}