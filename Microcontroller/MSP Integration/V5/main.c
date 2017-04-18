#include <msp430.h>
// Description: this is the fourth version, encorporating the motor and the current code
// Authors: Helen, Ahmed, and Chase
// Date last modified: 4/8/2017

#define ENABLE_PINS 0xFFFE  // Required to use inputs and outputs
#define UART_CLK_SEL 0x0080 // Specifies accurate clock for UART peripheral
#define BR0_FOR_9600 0x05   // Value required to use 9600 baud
#define BR1_FOR_9600 0x00   // Value required to use 9600 baud
#define MCTLW_9600	 0xB791 // Value required for 9600 baud
#define BR0_FOR_4800 0x0B    // Value required to use 4800 baud
#define BR1_FOR_4800 0x00   // Value required to use 4800 baud
#define MCTLW_4800	 0x9231 // Value required for 9600 baud
#define CLK_MOD 0x4911      // Microcontroller will "clean-up" clock signal
#define SMCLK           0x0200      // Timer SMCLK source
#define ACLK            0x0100		// Assume 32 kHz = 31.25us
#define UP              0x0010      // Timer Up mode
#define TAIFG           0x0001      // Used to look at Timer A Interrupt FlaG

#define MOTOR_OUT BIT3      // This is the pin for the motor
#define MOTOR_COUNT_2_SECS 0xFFFF	// Count for two seconds
#define MOTOR_COUNT_1_SEC  0x8000	// Count for one second
#define MOTOR_COUNT_50_DUTY 0x0280	// Motor on for 20ms = 50Hz
#define MOTOR_ITERATION 100			// Number of timer iterations for 50% duty
#define MOTOR_ITERATION_4_SECS 2		// Number of timer iterations for 4 seconds


void select_clock_signals(void); // Assigns microcontroller clock signals
void assign_pins_to_uart(void); // Assign pins to UART for Bluetooth and GPS
void assign_baud_rate(void); // assign baud rates for GPS and Bluetooth
void initialize_timers( void ); // Initialize Timers
unsigned int timeHighLow(signed int tHigh); // Used for specifying high and low time
void enable_inputs_and_outputs(void);       // an input or an output
void clear_timer0_elapsed_flag(void);       // clears timer flag
unsigned int timer0_elapsed(void);      // value of 0x01 when timer has elapsed
void timer0_will_count_up_for_300ns(void);  // Setup timer
void timerA0_count_for_300n(void);
void timerA0_in_up_mode(void);
unsigned int motorState(unsigned int m);

// global variables
int nColor = 0;
int nAlertState = 0;

int main(void)
 {

    WDTCTL = WDTPW | WDTHOLD; // Stop WDT

    enable_inputs_and_outputs();


    // Declare pins
    P1DIR = BIT0 | BIT1 | BIT2 | MOTOR_OUT;             // Make P1.0, P1.1, P1.2 outputs for LEDs
    P1OUT = P1OUT & ~(BIT0 | BIT1 | BIT2 | MOTOR_OUT);  // LEDs initially off

    select_clock_signals(); // Assigns microcontroller clock signals
    assign_pins_to_uart();  // Assign pins to UART for Bluetooth and GPS
    assign_baud_rate();         // UART operates at 9600 bits/second

    initialize_timers();

    UCA1IE = UCRXIE; // Enable UART RXD interrupt for Bluetooth
    // Timer A0
    TA0CCTL0 = CCIE;
    TA0CTL = ACLK | UP;
    TA0CCR0 =  20000;
    motorState(0);		// initialize motor state to off

    // Timer for Motor
/*   TA1CCTL0 = CCIE;
*    TA1CTL = ACLK | UP;
*    TA1CCR0 = 0x0500; // do not start counting yet
*/

    _BIS_SR(GIE);     // Activate enabled UART RXD interrupt

     // Declare variables
    char sMessage[1000];
    char* pMessage = sMessage;

    while(1){
        // Wait for GPS to send data over UART
        while(!(UCA0IFG & UCRXIFG)){
        	//UCA0TXBUF = 0xAA; // Send the UART message 0x56 out pin P4.2
        }
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
// Bluetooth Interrupt
#pragma vector=USCI_A1_VECTOR
__interrupt void UART_ISR(void)
{
    if(UCA1RXBUF == 0x47) // if message is "G" for green
    {
        nColor = 1;
        P1OUT = 0x00;
        P1OUT |= BIT0; // Turn on the green LED
        P1OUT= P1OUT & ~(MOTOR_OUT); // turn motor off
        motorState(0); 				 // set motor state to off
    }
    else if(UCA1RXBUF == 0x4F) // if message is "O" for orange
    {
        nColor = 2;
        P1OUT = 0x00;
        P1OUT |= BIT1; // Turn on the orange LED
        P1OUT= P1OUT | MOTOR_OUT; 		// turn motor on
        motorState(1);				 	// set motor state to warning
        TA0CCR0 = MOTOR_COUNT_50_DUTY; 	// set counter for 50Hz
    }
    else if(UCA1RXBUF == 0x52) // if message is "R" for red
    {
        nColor = 3;
        P1OUT = 0x00;
        P1OUT |= BIT2; // Turn on the red LED
        P1OUT= P1OUT | MOTOR_OUT; 		// turn motor on
        motorState(2);				 	// set motor state to danger
        TA0CCR0 = MOTOR_COUNT_2_SECS; 	// set counter for 2s
    }
    else if(UCA1RXBUF == 0x41) // if message is "A" for alert
    {
		P1OUT= P1OUT | MOTOR_OUT; 		// turn motor on
		motorState(3);				 	// set motor state to alert
		TA0CCR0 = MOTOR_COUNT_2_SECS; 	// set counter for 2s
		nAlertState = 1;				// alert on
    }
    else if(UCA1RXBUF == 0x42)	// if message is "B" for alert off
    {
		P1OUT= P1OUT & ~(MOTOR_OUT); 	// turn motor off
		motorState(0);				 	// set motor state off
		TA0CCR0 = 0x0000; 				// turn counter off
		nAlertState = 0;				// alert off
    }
    UCA1IFG = UCA1IFG & (~UCRXIFG); // Clear RX Interrupt FlaG
}
/*	Final Design
// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_ISR (void)
{
    static int LED_count=0;

    // if alert is on, toggle appropriate LED
    if(nAlertState == 1)
    {
        if( LED_count == 15 )
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
    }
    else // if alert is off, turn appropriate LED
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
    LED_count++;
    if( LED_count == 16 )
    {
        LED_count == 0;
    }
}
*/
// Timer A1-Motor interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void MOTOR_ISR (void)
{
	static unsigned int nTimerCount = 0;
	unsigned int nState;
	nState = motorState(4);
	if(nState == 1) // motor is in warning mode
	{
		if(nTimerCount >= MOTOR_ITERATION ) // if counter has counted to 2 seconds
		{
			P1OUT= P1OUT & (~MOTOR_OUT); // turn motor off
			motorState(0);				 // turn motor state off
			TA0CCR0 = 0x0000;			 // turn timer off
			nTimerCount = 1;			 // reset counter
		}
		else
		{
			P1OUT ^= MOTOR_OUT;	// toggle motor
			nTimerCount += 1;	// increment counter
		}
	}
	else if(nState == 2) // motor is in danger mode
	{
		if(nTimerCount >= MOTOR_ITERATION_4_SECS ) // if counter has counted to 4 seconds
		{
			P1OUT= P1OUT & (~MOTOR_OUT); // turn motor off
			motorState(0);				 // turn motor state off
			TA0CCR0 = 0x0000;			 // turn timer off
			nTimerCount = 1;			 // reset counter
		}
		else
		{
			nTimerCount += 1;	// increment counter
		}
	}
	else if(nState == 3) // motor is in alert mode
	{
		if( TA0CCR0 == MOTOR_COUNT_2_SECS) // if timer finished counting 2 seconds
		{
			P1OUT= P1OUT & (~MOTOR_OUT);  // turn motor off
			TA0CCR0 = MOTOR_COUNT_1_SEC; // set counter for 1 second
		}
		else	// if timer finished counting 1 second
		{
			P1OUT= P1OUT | MOTOR_OUT;  		// turn motor on
			TA0CCR0 = MOTOR_COUNT_2_SECS;  // set counter for 2 seconds
		}
	}
}

unsigned int motorState(unsigned int m)
{
	// State Index
	/*
	 *  Warning = 1
	 *  Danger = 2
	 *  Alert = 3
	 *  off = 0
	 *  read input only, do not write = 4
	 */
	unsigned static int nState = 0; // initialize as off

	if(m != 4)
	{
		nState = m;
	}
	return nState;
}

//*********************************************************************************
//*********************************************************************************
//* Select Clock Signals *
//*********************************************************************************

// Initialize Timers
void initialize_timers( void )  // this function initializes the timers
{
    timer0_will_count_up_for_300ns();
    return;
}

void select_clock_signals(void)
{
	// Main clock speed
	 CSCTL0 = 0x0000; //
	 CSCTL1 = 0x0000; // run frequency at 1 MHz
	 CSCTL2 = 0x0001; // do not use multipliers or dividers
	 CSCTL3 = 0x0000; // Use clocks at intended frequency, do not slow them down
}

//*********************************************************************************
//* Used to Give UART Control of Appropriate Pins *
//*********************************************************************************

void assign_pins_to_uart(void)
{
    // Assign UART pins for GPS
     P1SEL1 = 0x00;         // 0000 0000
     P1SEL0 = BIT4 | BIT5;  // 0011 0000
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
     UCA0CTLW0 = UCSWRST;                   // Put UART into SoftWare ReSeT
     UCA0CTLW0 = UCA0CTLW0 | UART_CLK_SEL;  // Specifies clock sourse for UART (SMCLK)
     UCA0BR0 = BR0_FOR_4800;                // Specifies bit rate (baud) of 4800
     UCA0BR1 = BR1_FOR_4800;                // Specifies bit rate (baud) of 4800
     UCA0MCTLW = MCTLW_4800;//CLK_MOD;                   // "Cleans" clock signal
     UCA0CTLW0 = UCA0CTLW0 & (~UCSWRST);    // Takes UART out of SoftWare ReSeT

     // UART for Bluetooth, 9600 baud rate
     UCA1CTLW0 = UCSWRST;                   // Put UART into SoftWare ReSeT
     UCA1CTLW0 = UCA1CTLW0 | UART_CLK_SEL;  // Specifies clock sourse for UART
     UCA1BR0 = BR0_FOR_9600;                // Specifies bit rate (baud) of 9600
     UCA1BR1 = BR1_FOR_9600;                // Specifies bit rate (baud) of 9600
     UCA1MCTLW = MCTLW_9600;//CLK_MOD;                   // "Cleans" clock signal
     UCA1CTLW0 = UCA1CTLW0 & (~UCSWRST);    // Takes UART out of SoftWare ReSeT
}



//*********************************************************************************
//* Enable Inputs and Outputs *
//*********************************************************************************

void enable_inputs_and_outputs(void)
{
    PM5CTL0 = ENABLE_PINS;      // Enables inputs and outputs
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
    return TA0CTL & TAIFG;      // This takes the bit-wise logic AND of
                                // the value we defined for TAIFG
                                // TAIFG = 0x0001 = 0000 0000 0000 0001
                                // and the contents of the TA1CTL register
                                // The result will be returned as the output
                                // back to the main program
                                // 0x0000 If TAIFG is LO and the timer has
                                // not yet counted up to TA0CCR0
                                // 0x0001 If TAIFG is HI and the timer has
                                // counted up to TA0CCR0
}
//*********************************************************************************
//*********************************************************************************
void timer0_will_count_up_for_300ns(void)
{
    timerA0_count_for_300n();   // SMCLK will count to 300ns
    timerA0_in_up_mode();
}
//*********************************************************************************
void timerA0_count_for_300n(void)
{
    //TA0CCR0 = 0xFFFF;            // 7 * 1/24MHz = 0.292us
}
//*********************************************************************************
void timerA0_in_up_mode(void)
{
    TA0CTL = UP | SMCLK;    // timer A0 count up with SMCLK
}
//*********************************************************************************
