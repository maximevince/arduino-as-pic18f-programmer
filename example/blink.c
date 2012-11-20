#include <pic18f2550.h>

unsigned int i;

void main (void)
{
	TRISA = 0;
	TRISB = 0;
	TRISC = 0;

  while (1)
	{
		LATA = 255;
		LATB = 255;
		LATC = 255;
		for(i=0;i<16000;i++);
		LATA = 0;
		LATB = 0;
		LATC = 0;
		for(i=0;i<16000;i++);
  }
}
