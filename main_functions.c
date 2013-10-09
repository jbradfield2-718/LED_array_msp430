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
volatile uint8_t row = 0;
volatile uint16_t highbytes = 0x8811;
volatile uint16_t lowbytes = 0xffff;

volatile char SCFG=0;    // flags for serial communication: BIT1 is UART RX in process,
				// BIT2 is SPI TX in process


void setup()
{
	WDTCTL = WDTPW + WDTHOLD;                 				// Stop WDT
	BCSCTL1 |= 15;											// SET RSELx to15
	DCOCTL |= BIT5 + BIT6;									// SET DCOx to 3...should set DCO ~16Mhz...no oscilloscope. :(
	CCTL0 = CCIE;                             				// CCR0 interrupt enabled
	TACTL = TASSEL_2 + MC_1 + ID_0;           				// SMCLK no division, upmode
	CCR0 =  5000;                             				// Assuming DCO is running at 16MHz, throws interrupt every 312uS, approx 3000Hz
	P1DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4;              // P1.0 - P1.4 output
	P1OUT &= 0x00;

	// Setup SPI as master at SMCLK=DCO
	USICTL0 |= USIPE6 + USIPE5 + USILSB + USIMST + USIOE; 	// Use pins 5 (sclk), 6 sdo, SPI master
	USICNT |= USI16B;										// Select for transmission of 16bits in SPI
	USICKCTL = USIDIV_0 + USISSEL_2;          				// /1 SMCLK
	USICTL0 &= ~USISWRST;                     				// USI released for operation

	USISR = 0;
	USICNT = 16;                               				// init-load counter

	_BIS_SR(GIE);                   			 // Enter LPM0 w/ interrupt
}

void rowselect(int curr_row)
{
	curr_row = curr_row << 1;					// Bitwise shift left, as Bit0 is used as latch (probably bad practice)...
	P1OUT = curr_row;
}
	




// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	rowselect(row);
	if(row <= NUMROWS)
	{row++;}
	else
	{row = 0;}								//


	USISR = lowbytes;						// LOADS
	USICNT |= USI16B + 16;								// SENDS
	//__delay_cycles(50);
	while ( ! (USIIFG & USICTL1) );			// Waits until lowbyte shifted in


	USISR = highbytes;						// LOADS
	USICNT |= USI16B + 17;					// SENDS.  Needs the extra bit or LED pattern is incorrect, off by 1!
	//__delay_cycles(50);
	while ( ! (USIIFG & USICTL1) );			// Waits until highbyte shifted in


	 P1OUT |= BIT0;								// Pulses pin P1.0 TO latch in data to stp08dp05
	 P1OUT &= ~BIT0;
	 USICTL1 &= ~USIIFG;


}

