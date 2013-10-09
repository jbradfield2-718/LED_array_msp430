/*
 * main_functions.c
 *
 * Created: 9/2/2013 3:03:49 PM
 *  Author: Turing
 */

#include <msp430g2231.h>
#include "stdint.h"
#include "main_functions.h"


volatile uint8_t swap_flag = 0;
volatile uint8_t byte1 = 0xAA;
volatile uint8_t byte2 = 0x88;

volatile char SCFG=0;    // flags for serial communication: BIT1 is UART RX in process,
				// BIT2 is SPI TX in process


void setup()
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	BCSCTL1 |= 15;								// SET RSELx to15
	DCOCTL |= BIT5 + BIT6;						// SET DCOx to 3...should set DCO ~16Mhz
	CCTL0 = CCIE;                             // CCR0 interrupt enabled
	TACTL = TASSEL_2 + MC_1 + ID_3;           // SMCLK/8, upmode
	CCR0 =  5000;                             //
	//P1SEL |= BIT5 + BIT6;
	P1DIR |= BIT0 + BIT5 + BIT6;              // P1.0 and P1.1 pins output, the rest are input
	P1OUT &= 0x00;                        					// Shut. Down. Everything... :)

	// Setup SPI as master at SMCLK=DCO
	USICTL0 |= USIPE6 + USIPE5 + USILSB + USIMST + USIOE; 	// Use pins 5 (sclk), 6 sdo, SPI master
	//USICTL1 |= USIIE;                         			// Counter interrupt, flag remains set...I don't think I need interrupt for xmit only
	USICKCTL = USIDIV_0 + USISSEL_2;          				// /1 SMCLK
	USICTL0 &= ~USISWRST;                     				// USI released for operation

	USISR = 0;
	USICNT = 8;                               				// init-load counter

	_BIS_SR(GIE);                   			 // Enter LPM0 w/ interrupt
}
	




// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	//uint8_t counter = 0;

	//while (counter < 2)
	//{

	//while ( ! ( USIIFG ) ) ;
	//{
	if(swap_flag == 0)
		{USISR = byte1;}						// LOADS
	else if(swap_flag ==1)
		{USISR = byte2;}
		USICNT = 8;							// SENDS
		USISR = 0x00;
	//}
	//}

	 P1OUT |= BIT0;								// Pulses pin P1.0 TO latch in data to stp08dp05
	 P1OUT &= ~BIT0;
	 USICTL1 &= ~USIIFG;

	 if(swap_flag == 0)
	 {swap_flag = 1;}
	 else
	 {swap_flag = 0;}


}

/*
// USI interrupt service routine
#pragma vector=USI_VECTOR
__interrupt void universal_serial_interface(void)
{
  P1OUT |= 0x02;                            // Disable TLC549
  if (USISRL > 0x7F)
    P1OUT |= 0x01;
  else
    P1OUT &= ~0x01;
  P1OUT &= ~0x02;                            // Enable TLC549
  USICNT = 8;                                // re-load counter
}*/
