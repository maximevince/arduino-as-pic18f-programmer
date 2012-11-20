#include <pic18f2550.h>

__code char __at __CONFIG1H CONFIG1H = _OSC_INTOSC__INTOSC_CLK0_RA6___USB_EC_1H;

int main(void) 
{ 
    int i;
    TRISA = 0x00;
    TRISB = 0x00;
    TRISC = 0x00;
    while(1) 
    { 
      LATA ^= 0xFF;
      LATB ^= 0xFF;
      LATC ^= 0xFF;  
      for(i=0; i < 100000; i++); 
    }
    return 0;
}