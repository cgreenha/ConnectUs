#include <msp430.h>

#define ENABLE_PINS 0xFFFE 	// Required to use inputs and outputs
#define UART_CLK_SEL 0x0080 // Specifies accurate clock for UART peripheral
#define BR0_FOR_9600 0x41 	// Value required to use 9600 baud
#define BR1_FOR_9600 0x00 	// Value required to use 9600 baud
#define BR0_FOR_4800 0x83 	// Value required to use 4800 baud
#define BR1_FOR_4800 0x00 	// Value required to use 4800 baud
#define CLK_MOD 0x4911 		// Microcontroller will "clean-up" clock signal
#define SMCLK			0x0200  	// Timer SMCLK source
#define ACLK 			0x0100
#define UP              0x0010  	// Timer Up mode
#define TAIFG 			0x0001 		// Used to look at Timer A Interrupt FlaG

void select_clock_signals(void); // Assigns microcontroller clock signals
void assign_pins_to_uart(void); // Assign pins to UART for Bluetooth and GPS
void assign_baud_rate(void); // assign baud rates for GPS and Bluetooth
void initialize_timers( void );	// Initialize Timers
unsigned int timeHighLow(signed int tHigh); // Used for specifying high and low time
void enable_inputs_and_outputs(void); 		// an input or an output
void clear_timer0_elapsed_flag(void);		// clears timer flag
unsigned int timer0_elapsed(void);		// value of 0x01 when timer has elapsed
void timer0_will_count_up_for_300ns(void);	// Setup timer
void timerA0_count_for_300n(void);
void timerA0_in_up_mode(void);
void rgb(char RED, char GREEN, char BLUE)  // use to set up RGB function
{
	P4OUT = RED << 1 | GREEN << 2 | BLUE << 3;  // shifted to the correct position and the register
	                                            //P4OUT = 0b00001110; // turn them	                                            // and OR them altogether
}

// global variables
int nColor = 3;
int nAlertState = 1;

int main(void)
{

	WDTCTL = WDTPW | WDTHOLD; // Stop WDT

	enable_inputs_and_outputs();


	// Declare pins
	P4DIR = 0b00001110;     // P4.1, P4.2, P4.3 as output
	                        // P4OUT = 0b00001110     turn them on
	
	select_clock_signals(); // Assigns microcontroller clock signals
	assign_pins_to_uart(); 	// Assign pins to UART for Bluetooth and GPS
	assign_baud_rate(); 	// UART operates at 9600 bits/second

	initialize_timers();

	UCA1IE = UCRXIE; // Enable UART RXD interrupt for Bluetooth
	_BIS_SR(GIE); 	  // Activate enabled UART RXD interrupt
	TA0CCTL0 = CCIE;
	TA0CTL = ACLK | UP;
	TA0CCR0 =  1500;


	 // Declare variables
	char sMessage[1000];
	char* pMessage = sMessage;

	while(1){
		// Wait for GPS to send data over UART
		while(!(UCA0IFG & UCRXIFG))
		UCA0IFG = UCA0IFG & (~UCRXIFG); // Clear RX Interrupt FlaG
		*pMessage = UCA0RXBUF;
		pMessage++;
		if(pMessage>&sMessage[999])
		{
			pMessage = sMessage;
		}
	}
}


//*********************************************************************************
//* UART RX Interrupt *
//*********************************************************************************

#pragma vector=USCI_A1_VECTOR
__interrupt void UART_ISR(void)
{
	if(UCA1RXBUF == 0x47) // if message is "G" for green
	{
		nColor = 1;
		P4DIR = 0b00001110;
	}
	else if(UCA1RXBUF == 0x4F) // if message is "O" for orange
	{
		nColor = 2;
		P4DIR = 0b00001110;
	}
	else if(UCA1RXBUF == 0x52) // if message is "R" for red
	{
		nColor = 3;
		P4DIR = 0b00001110;
	}
	else if(UCA1RXBUF == 0x41) // if message is "A" for alert
	{
		if(nAlertState == 0) // if alert state is off
		{
			nAlertState = 1; // turn alert on
		}
		else // if alert state is on
		{
			nAlertState = 0; // turn alert off
		}
	}
	UCA1IFG = UCA1IFG & (~UCRXIFG); // Clear RX Interrupt FlaG
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_ISR (void)
{
	// if alert is on, toggle appropriate LED
	if(nAlertState == 1)
	{
		if(nColor == 1)
		{
			rgb(0,1,0); // turn on green LED
			__delay_cycles(1500000);
		}
		else if(nColor == 2)
		{
			long i = 0;			//turn on the yellow LED
			    	for(i=0; i<1500;i++)
			    	{
			    		rgb(1,1,0); // yellow
			    		__delay_cycles(110);
			    		rgb(1,0,0); // red
			    		__delay_cycles(820);
			    	}
		}
		else if(nColor == 3)
		{
			rgb(1,0,0); // Turn on the red LED
			__delay_cycles(1500000);
		}
	}
	else // if alert is off, turn appropriate LED
	{
		if(nColor == 1)
		{

			rgb(0,0,0); // // Turn off the green LED
			__delay_cycles(1500000);
		}
		else if(nColor == 2)
		{

			rgb(0,0,0); // // Turn off the Yellow LED
			__delay_cycles(1500000);
		}
		else if(nColor == 3)
		{

			rgb(0,0,0); // // Turn off the red LED
			__delay_cycles(1500000);
		}
	}
}

//*********************************************************************************
//*********************************************************************************
//* Select Clock Signals *
//*********************************************************************************

// Initialize Timers
void initialize_timers( void )	// this function initializes the timers
{
	timer0_will_count_up_for_300ns();
	return;
}

void select_clock_signals(void)
{
	 CSCTL0 = 0xA500; // "Password" to access clock calibration registers
	 CSCTL1 = 0x0046; // Specifies frequency of main clock
	 CSCTL2 = 0x0133; // Assigns additional clock signals
	 CSCTL3 = 0x0000; // Use clocks at intended frequency, do not slow them down
}

//*********************************************************************************
//* Used to Give UART Control of Appropriate Pins *
//*********************************************************************************

void assign_pins_to_uart(void)
{
	// Assign UART pins for GPS
	 P1SEL1 = 0x00; 		// 0000 0000
	 P1SEL0 = BIT4 | BIT5; 	// 0011 0000
							// ^^
							// ||
							// |+---- 01 assigns P1.4 to UART Transmit (TXD)
							// |
							// +----- 01 assigns P1.5 to UART Receive (RXD)

	 P2SEL1 = 0x00;
	 P2SEL0 = BIT5 | BIT6;  // P2.5 to UART RXD
	 	 	 	 	 	 	// P2.6 to UART TXD
}

//*********************************************************************************
//* Specify UART Baud Rate *
//*********************************************************************************

void assign_baud_rate(void)
{
	// UART for GPS, 4800 baud rate
	 UCA0CTLW0 = UCSWRST; 					// Put UART into SoftWare ReSeT
	 UCA0CTLW0 = UCA0CTLW0 | UART_CLK_SEL; 	// Specifies clock sourse for UART
	 UCA0BR0 = BR0_FOR_4800; 				// Specifies bit rate (baud) of 4800
	 UCA0BR1 = BR1_FOR_4800; 				// Specifies bit rate (baud) of 4800
	 UCA0MCTLW = CLK_MOD; 					// "Cleans" clock signal
	 UCA0CTLW0 = UCA0CTLW0 & (~UCSWRST); 	// Takes UART out of SoftWare ReSeT

	 // UART for Bluetooth, 9600 baud rate
	 UCA1CTLW0 = UCSWRST; 					// Put UART into SoftWare ReSeT
	 UCA1CTLW0 = UCA1CTLW0 | UART_CLK_SEL; 	// Specifies clock sourse for UART
	 UCA1BR0 = BR0_FOR_9600; 				// Specifies bit rate (baud) of 9600
	 UCA1BR1 = BR1_FOR_9600; 				// Specifies bit rate (baud) of 9600
	 UCA1MCTLW = CLK_MOD; 					// "Cleans" clock signal
	 UCA1CTLW0 = UCA1CTLW0 & (~UCSWRST); 	// Takes UART out of SoftWare ReSeT
}



//*********************************************************************************
//* Enable Inputs and Outputs *
//*********************************************************************************

void enable_inputs_and_outputs(void)
{
	PM5CTL0 = ENABLE_PINS; 		// Enables inputs and outputs
}


//*********************************************************************************
void clear_timer0_elapsed_flag(void)
{
	TA0CTL = TA0CTL & (~TAIFG); // Like we have seen before, this first looks
								// at the value of TAIFG which we defined:
								// TAIFG = 0x0001 = 0000 0000 0000 0001
								// Then, it bit-wise inverts the value
								// ~TAIFG = 0xFFFE = 1111 1111 1111 1110
								// Then, it bit-wise ANDs the 0xFFFE value with
								// the contents of TA1CTL. This clears the
								// TAIFG bit (bit 0 of TA1CTL) without
								// modifying any of the other bits
}
//*********************************************************************************
unsigned int timer0_elapsed(void)
{
	return TA0CTL & TAIFG; 		// This takes the bit-wise logic AND of
								// the value we defined for TAIFG
								// TAIFG = 0x0001 = 0000 0000 0000 0001
								// and the contents of the TA1CTL register
								// The result will be returned as the output
								// back to the main program
								// 0x0000 If TAIFG is LO and the timer has
								// not yet counted up to TA1CCR0
								// 0x0001 If TAIFG is HI and the timer has
								// counted up to TA1CCR0
}
//*********************************************************************************
//*********************************************************************************
void timer0_will_count_up_for_300ns(void)
{
	timerA0_count_for_300n();	// SMCLK will count to 300ns
	timerA0_in_up_mode();
}
//*********************************************************************************
void timerA0_count_for_300n(void)
{
	TA0CCR0 = 7;			// 7 * 1/24MHz = 0.292us
}
//*********************************************************************************
void timerA0_in_up_mode(void)
{
	TA0CTL = UP | SMCLK;	// timer A0 count up with SMCLK
}
//*********************************************************************************
