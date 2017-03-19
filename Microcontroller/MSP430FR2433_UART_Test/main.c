#include <msp430.h>

#define ENABLE_PINS 0xFFFE // Required to use inputs and outputs
#define UART_CLK_SEL 0x0080 // Specifies accurate clock for UART peripheral
#define BR0_FOR_9600 0x41 // Value required to use 9600 baud
#define BR1_FOR_9600 0x00 // Value required to use 9600 baud
#define BR0_FOR_4800 0x83 //64 // Value required to use 4800 baud
#define BR1_FOR_4800 0x00 // Value required to use 4800 baud
#define CLK_MOD 0x4911 // Microcontroller will "clean-up" clock signal
#define TIMER_208US  	208				// NCount for SMCLK 50MS
#define SMCLK			0x0200  			// Timer SMCLK source
#define ACLK 			0x0100
#define UP              0x0010  			// Timer Up mode
#define TAIFG 			0x0001 			// Used to look at Timer A Interrupt FlaG

void select_clock_signals(void); // Assigns microcontroller clock signals
void assign_pins_to_uart(void); // P4.2 is for TXD, P4.3 is for RXD
void use_9600_baud(void); // UART operates at 9600 bits/second
void initialize_timers( void );	// Initialize Timers
unsigned int timeHighLow(signed int tHigh); // Used for specifying high and low time
void enable_inputs_and_outputs(void); 		// an input or an output
void turnGreen(void);						// turns neopixel green
void turnOrange(void);
void turnRed(void);
void sendLED0(void);						// sends a binary 0 to neopixel
void sendLED1(void);						// sends a binary 1 to neopixel
void clear_timer0_elapsed_flag(void);		// clears timer flag
unsigned int timer0_elapsed(void);		// value of 0x01 when timer has elapsed
void timer0_will_count_up_for_300ns(void);	// Setup timer
void timerA0_count_for_300n(void);
void timerA0_in_up_mode(void);
void wait300ns(void);

// global variables
int nColor = 0;
int nAlertState = 0;

int main(void)
{

	 WDTCTL = WDTPW | WDTHOLD; // Stop WDT

	 enable_inputs_and_outputs();



	 P1DIR = BIT0 | BIT1 | BIT2; // Make P1.0, P1.1, P1.2 outputs for LEDs
	 P1OUT = P1OUT & ~(BIT0 | BIT1 | BIT2); // LEDs initially off

	 select_clock_signals(); // Assigns microcontroller clock signals
	 assign_pins_to_uart(); // P4.2 is for TXD, P4.3 is for RXD
	 use_9600_baud(); // UART operates at 9600 bits/second

	 initialize_timers();


	 //UCA0IE = UCRXIE; // Enable UART RXD interrupt
	 UCA1IE = UCRXIE; // Enable UART RXD interrupt
	 _BIS_SR(GIE); 	  // Activate enabled UART RXD interrupt
	 TA0CCTL0 = CCIE;
	 TA0CTL = ACLK | UP;
	 TA0CCR0 =  1500;

	 //UCA0TXBUF = 0x56; // Send the UART message 0x56 out pin P4.2

	 // Declare variables
	 	char sMessage[1000];
	 	char* pMessage = sMessage;

	 	while(1){
	 		while(!(UCA0IFG & UCRXIFG))
	 		UCA0IFG = UCA0IFG & (~UCRXIFG); // Clear RX Interrupt FlaG
	 		*pMessage = UCA0RXBUF;
	 		pMessage++;
	 		if(pMessage>&sMessage[999])
	 		{
	 			pMessage = sMessage;
	 		}
	 	}

	/* while(1)
		 {
		 UCA0TXBUF = 0xAA; // Send the UART message 0x56 out pin P4.2
		 //turnGreen(); // Wait here unless you get UART interrupt
		 //turnRed();
		 //turnOrange();
		 }*/
}

//*********************************************************************************
//* UART RX Interrupt *
//*********************************************************************************
/*
#pragma vector=USCI_A0_VECTOR
__interrupt void UART_ISR(void)
{
	if(UCA0RXBUF == 0x47) // Check to see if the message is "G"
	{
		nColor = 1;
		P1OUT = 0x00;
		//turnGreen();
	}
	else if(UCA0RXBUF == 0x4F) // "O"
	{
		nColor = 2;
		P1OUT = 0x00;
		//turnOrange();
	}
	else if(UCA0RXBUF == 0x52) // "R"
	{
		nColor = 3;
		P1OUT = 0x00;
		//turnRed();
	}
	else if(UCA0RXBUF == 0x41) // "A"
	{
		if(nAlertState == 0)
		{
			nAlertState = 1;
		}
		else
		{
			nAlertState = 0;
		}
	}
	UCA0IFG = UCA0IFG & (~UCRXIFG); // Clear RX Interrupt FlaG
}
*/
//*********************************************************************************
//* UART RX Interrupt *
//*********************************************************************************

#pragma vector=USCI_A1_VECTOR
__interrupt void UART_ISR(void)
{
	if(UCA1RXBUF == 0x47) // Check to see if the message is "G"
	{
		nColor = 1;
		P1OUT = 0x00;
		//turnGreen();
	}
	else if(UCA1RXBUF == 0x4F) // "O"
	{
		nColor = 2;
		P1OUT = 0x00;
		//turnOrange();
	}
	else if(UCA1RXBUF == 0x52) // "R"
	{
		nColor = 3;
		P1OUT = 0x00;
		//turnRed();
	}
	else if(UCA1RXBUF == 0x41) // "A"
	{
		if(nAlertState == 0)
		{
			nAlertState = 1;
		}
		else
		{
			nAlertState = 0;
		}
	}
	UCA1IFG = UCA1IFG & (~UCRXIFG); // Clear RX Interrupt FlaG
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_ISR (void)
{
	if(nAlertState == 1)
	{
		if(nColor == 1)
		{

			P1OUT ^= BIT0; // Turn on the green LED
		}
		else if(nColor == 2)
		{

			P1OUT ^= BIT1; // Turn on the orange LED
		}
		else if(nColor == 3)
		{

			P1OUT ^= BIT2; // Turn on the red LED
		}
	}
	else
	{
		if(nColor == 1)
		{

			P1OUT = BIT0; // Turn on the green LED
		}
		else if(nColor == 2)
		{

			P1OUT = BIT1; // Turn on the orange LED
		}
		else if(nColor == 3)
		{

			P1OUT = BIT2; // Turn on the red LED
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
	// Startup clock system with max DCO setting ~8MHz
	//CSCTL0 = CSKEY; // Unlock clock registers
	//CSCTL1 = DCOFSEL_6| DCORSEL; // Set DCO to 24MHz
	//// CSCTL1&=~DCORSEL;
	//CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
	//CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1; // Set all dividers
	//CSCTL0_H = 0; // Lock CS registers

	timer0_will_count_up_for_300ns();

	//TA1CCTL0	= CCIE;						//	Enable TA1 compare interrupt
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
	 P1SEL1 = 0x00; 		// 0000 0000
	 P1SEL0 = BIT4 | BIT5; 	// 0000 1100
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

void use_9600_baud(void)
{
	 UCA0CTLW0 = UCSWRST; 					// Put UART into SoftWare ReSeT
	 UCA0CTLW0 = UCA0CTLW0 | UART_CLK_SEL; 	// Specifies clock sourse for UART
	 UCA0BR0 = BR0_FOR_4800;//BR0_FOR_9600; 				// Specifies bit rate (baud) of 9600
	 UCA0BR1 = BR1_FOR_4800;//BR1_FOR_9600; 				// Specifies bit rate (baud) of 9600
	 UCA0MCTLW = CLK_MOD; 					// "Cleans" clock signal
	 UCA0CTLW0 = UCA0CTLW0 & (~UCSWRST); 	// Takes UART out of SoftWare ReSeT

	 UCA1CTLW0 = UCSWRST; 					// Put UART into SoftWare ReSeT
	 UCA1CTLW0 = UCA0CTLW0 | UART_CLK_SEL; 	// Specifies clock sourse for UART
	 UCA1BR0 = BR0_FOR_9600;//BR0_FOR_9600; 				// Specifies bit rate (baud) of 9600
	 UCA1BR1 = BR1_FOR_9600;//BR1_FOR_9600; 				// Specifies bit rate (baud) of 9600
	 UCA1MCTLW = CLK_MOD; 					// "Cleans" clock signal
	 UCA1CTLW0 = UCA0CTLW0 & (~UCSWRST); 	// Takes UART out of SoftWare ReSeT
}



//*********************************************************************************
//* Enable Inputs and Outputs *
//*********************************************************************************

void enable_inputs_and_outputs(void)
{
	PM5CTL0 = ENABLE_PINS; 		// Enables inputs and outputs
}

//*********************************************************************************
//* Turns the NeoPixel different colors *
//*********************************************************************************

void turnGreen(void)
{
	unsigned int i;
	for(i = 0; i < 8; i++)			// send binary for green color
	{
		sendLED1();
	}
	for(i = 0; i < 8; i++)			// send binary for red color
	{
		sendLED0();
	}
	for(i = 0; i < 8; i++)			// send binary for blue color
	{
		sendLED0();
	}
}

void turnRed(void)
{
	unsigned int i;
	for(i = 0; i < 8; i++)			// send binary for green color
	{
		sendLED0();
	}
	for(i = 0; i < 8; i++)			// send binary for red color
	{
		sendLED1();
	}
	for(i = 0; i < 8; i++)			// send binary for blue color
	{
		sendLED0();
	}
}

void turnOrange(void)
{
	unsigned int i;
	sendLED1();						// send binary for green color
	sendLED0();
	sendLED1();
	sendLED0();
	sendLED0();
	sendLED1();
	sendLED0();
	sendLED1();
	for(i = 0; i < 8; i++)			// send binary for red color
	{
		sendLED1();
	}
	for(i = 0; i < 8; i++)			// send binary for blue color
	{
		sendLED0();
	}
}

void sendLED0(void)
{
	/*P4OUT = P4OUT | BIT0; 		// Turn P4.0 on
	wait300ns();				// wait 300 ns
	P4OUT = P4OUT & ~(BIT0); 	// Turn P4.0 off
	wait300ns();				// wait 900 ns
	wait300ns();
	wait300ns();*/
}

void sendLED1(void)
{
	/*P4OUT = P4OUT | BIT0; 		// Turn P4.0 on
	wait300ns();				// wait 600 ns
	wait300ns();
	P4OUT = P4OUT & ~(BIT0); 	// Turn P4.0 off
	wait300ns();				// wait 600 ns
	wait300ns();*/
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
	timerA0_count_for_300n();	// ACLK will increment every 25us (0.000025s)
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
	TA0CTL = UP | SMCLK;
}
//*********************************************************************************

void wait300ns(void)
{
	clear_timer0_elapsed_flag();
	while(1)
	{
		if(timer0_elapsed())
		{
			clear_timer0_elapsed_flag();
			break;
		}
	}
}
