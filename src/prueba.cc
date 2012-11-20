#include <stdio.h>
#include "rs232.h"
#include "hex.h"
#include <stdlib.h>
#include <iostream>

using namespace std;


int main ()
{

	char buf[4096];
	int n;
	unsigned int address=0,i,j;
	int comPort=-1;

	//reading the hex file
	//printf("Enter location of hex file: ");
	//scanf("%4096[^\n]",buf);


	printf("---------------------------\n");
	printf("Connecting to the Arduino...");

	for(i=0;i<16;i++){
		if(!OpenComport(i, 9600)){
				cout << "Puerto abierto: " << i << endl;
				CloseComport(i);
				sleep(2);
				SendBuf(i,(unsigned char *)"DX",strlen("DX"));
			}

		}

	return 0;
}
