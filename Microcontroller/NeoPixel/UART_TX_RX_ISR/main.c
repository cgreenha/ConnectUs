#include <msp430.h>

#define ENABLE_PINS 0xFFFE // Required to use inputs and outputs
#define UART_CLK_SEL 0x0080 // Specifies accurate clock for UART peripheral
#define BR0_FOR_9600 0x34 // Value required to use 9600 baud
#define BR1_FOR_9600 0x00 // Value required to use 9600 baud
#define CLK_MOD 0x4911 // Microcontroller will "clean-up" clock signal
#define TIMER_208US  	208				// NCount for SMCLK 50MS
#define SMCLK			0x0200  			// Timer SMCLK source
#define UP              0x0010  			// Timer Up mode

void select_clock_signals(void); // Assigns microcontroller clock signals
void assign_pins_to_uart(void); // P4.2 is for TXD, P4.3 is for RXD
void use_9600_baud(void); // UART operates at 9600 bits/second
void initialize_timers( void );	// Initialize Timers
unsigned int timeHighLow(signed int tHigh); // Used for specifying high and low time

int main(void)
{

	 WDTCTL = WDTPW | WDTHOLD; // Stop WDT
	 PM5CTL0 = ENABLE_PINS; // Enable pins

	 P1DIR = BIT0; // Make P1.0 an output for red LED
	 P9DIR = BIT7 | BIT3; // Make P1.7 an output for green and orange LED
	 P4DIR = P4DIR | BIT0; // Make P4.0 an output
	 P1OUT = 0x00; // Red LED initially off
	 P9OUT = 0x00; // Green and orange LED initially off
	 P4OUT = P4OUT & ~(BIT0); // P4.0 initially off

	 initialize_timers();
	 select_clock_signals(); // Assigns microcontroller clock signals
	 assign_pins_to_uart(); // P4.2 is for TXD, P4.3 is for RXD
	 use_9600_baud(); // UART operates at 9600 bits/second

	 UCA0IE = UCRXIE; // Enable UART RXD interrupt
	 _BIS_SR(GIE); // Activate enabled UART RXD interrupt

	 UCA0TXBUF = 0x56; // Send the UART message 0x56 out pin P4.2

	 while(1); // Wait here unless you get UART interrupt
}

//*********************************************************************************
//* UART RX Interrupt *
//*********************************************************************************

#pragma vector=USCI_A0_VECTOR
__interrupt void UART_ISR(void)
{
	if(UCA0RXBUF == 0x47) // Check to see if the message is "G"
	{
		P1OUT = 0x00;
		P9OUT = 0x00;
		P9OUT = BIT7; // Turn on the green LED
		timeHighLow(4); // Duty Cycle 100%

	}
	else if(UCA0RXBUF == 0x4F) // "O"
	{
		P1OUT = 0x00;
		P9OUT = 0x00;
		P9OUT = BIT3; // Turn on the orange LED
		timeHighLow(2); // Duty Cycle 50%

	}
	else if(UCA0RXBUF == 0x52) // "R"
	{
		P1OUT = 0x00;
		P9OUT = 0x00;
		P1OUT = BIT0; // Turn on the red LED
		timeHighLow(1); // 25%

	}
	UCA0IFG = UCA0IFG & (~UCRXIFG); // Clear RX Interrupt FlaG
}

//**************************************************************************
// Interrupt Declaration
//**************************************************************************
#pragma vector = TIMER1_A0_VECTOR;			// Places ISR function
__interrupt void ToggleLED(void){	// Can rename ISR Timer_AO
	static signed int nCount = 1;
	if(nCount > timeHighLow(-1))
	{
		P4OUT = P4OUT & ~(BIT0);
	}
	else
	{
		P4OUT = P4OUT | BIT0;
	}
	if( nCount > 3)
	{
		nCount = 1;
	}
	else
	{
		nCount = nCount + 1;
	}
}	// End interrupt funtion IncrementDutyCycle()



//*********************************************************************************
//*********************************************************************************
//* Select Clock Signals *
//*********************************************************************************

// Initialize Timers
void initialize_timers( void ){	// this function initializes the timers
	TA1CCR0		= TIMER_208US;				// 	initial value for first cycle
	TA1CTL		= SMCLK | UP;				// 	Setup and start TimerA0 clock

	TA1CCTL0	= CCIE;						//	Enable TA1 compare interrupt
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
	 P4SEL1 = 0x00; 		// 0000 0000
	 P4SEL0 = BIT3 | BIT2; 	// 0000 1100
							// ^^
							// ||
							// |+---- 01 assigns P4.2 to UART Transmit (TXD)
							// |
							// +----- 01 assigns P4.3 to UART Receive (RXD)
}

//*********************************************************************************
//* Specify UART Baud Rate *
//*********************************************************************************

void use_9600_baud(void)
{
	 UCA0CTLW0 = UCSWRST; 					// Put UART into SoftWare ReSeT
	 UCA0CTLW0 = UCA0CTLW0 | UART_CLK_SEL; 	// Specifies clock sourse for UART
	 UCA0BR0 = BR0_FOR_9600; 				// Specifies bit rate (baud) of 9600
	 UCA0BR1 = BR1_FOR_9600; 				// Specifies bit rate (baud) of 9600
	 UCA0MCTLW = CLK_MOD; 					// "Cleans" clock signal
	 UCA0CTLW0 = UCA0CTLW0 & (~UCSWRST); 	// Takes UART out of SoftWare ReSeT
}

//*********************************************************************************
//* Specify high and low time *
//*********************************************************************************

unsigned int timeHighLow(signed int tHigh) // Used for specifying high and low time
{
	static unsigned int tH = 4;
	if((tHigh < 5) && (tHigh > -1))
	{
		tH = tHigh;
	}
	return tH;
}
