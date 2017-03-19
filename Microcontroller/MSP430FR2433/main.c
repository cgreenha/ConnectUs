#include <msp430.h> 

/*
 * main.c
 */
#define   ENABLE_PINS   0xFFFE      // Required to use inputs and outputs


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    PM5CTL0 = ENABLE_PINS;          // Enable inputs and outputs
    P1DIR   = P1DIR | BIT0;                 // P1.0 will be output for red LED
	while(1)
	{
		long i = 0;
		for(i=0;i<123456; i++);
		P1OUT = P1OUT ^ BIT0;
	}
	//return 0;
}
