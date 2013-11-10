/*
 * main_functions.c
 *
 * Created: 9/2/2013 3:03:49 PM
 *  Author: Turing
 */

//#include <msp430.h>
#include "msp430g2533.h"
#include "stdint.h"
#include <stdlib.h>
#include "main_functions.h"
#include "string.h"
#include "math.h"


volatile int one_sec_flag = 0;
volatile int swap_array_flag = 0;
volatile uint8_t reset_game_flag = 0;
volatile uint16_t count = 0;
volatile uint8_t row = 0;

volatile uint8_t lowbyte = 0x00;
volatile uint8_t medlowbyte = 0x00;
volatile uint8_t medhighbyte = 0x00;
volatile uint8_t highbyte = 0x00;
volatile uint8_t colon_blink = 0;
volatile uint8_t am_pm = 0;							// Set to 0 for am, pm = 1
volatile uint8_t num = 0;							// This is a variable for testing array

/* 	Variables to hold the current time.  Clock only works with am/pm system 12 hr format.  To save updating entire array whenever time changes,
	I hold both the current and last (updated every second) state of the time.  When current digit does not match last state, update_clock function
	is called to update that porition of the array only. !! CURRENTLY NOT USED !!
*/
volatile uint8_t tens_hrs = 1;
//volatile uint8_t tens_hrs_last = 1;
volatile uint8_t ones_hrs = 2;
//volatile uint8_t ones_hrs_last = 2;
volatile uint8_t tens_mins = 0;
//volatile uint8_t tens_mins_last = 0;
volatile uint8_t ones_mins = 0;
//volatile uint8_t ones_mins_last = 0;

volatile uint32_t array[NUMROWS +1] =
{
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000

};

volatile uint32_t nextarray[NUMROWS +1] =
{
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000

};

// These are the numbers which will be displayed to the screen during clock mode.
// Hex values...each digit 12 pixels high, 5 pixels wide.  Store each row as a byte.
const uint8_t zero[12] =
{
		0x0e, 0x11, 0X11, 0X11, 0X11, 0X11, 0X11, 0X11, 0X11, 0X11, 0X11, 0X0e
};
const uint8_t one[12] =
{
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};
const uint8_t two[12] =
{
		0X1f, 0X10, 0X10, 0X10, 0X08, 0X04, 0X02, 0X01, 0X01, 0X11, 0x11, 0x0E
};
const uint8_t three[12] =
{
		0x0E, 0x11, 0X11, 0X01, 0X01, 0X01, 0X06, 0X01, 0X01, 0X11, 0X11, 0X0E
};
const uint8_t four[12] =
{
		0x01, 0x01, 0X01, 0X01, 0X01, 0X01, 0X1f, 0X11, 0X11, 0X11, 0X11, 0X01
};
const uint8_t five[12] =
{
		0x0e, 0x11, 0X11, 0X01, 0X01, 0X01, 0X01, 0X1e, 0X10, 0X10, 0X10, 0X1f
};
const uint8_t six[12] =
{
		0x0e, 0x11, 0X11, 0X11, 0X11, 0X1e, 0X10, 0X10, 0X10, 0X10, 0X10, 0X0e
};
const uint8_t seven[12] =
{
		0x04, 0x04, 0X04, 0X04, 0X02, 0X02, 0X02, 0X01, 0X01, 0X01, 0X01, 0X1f
};
const uint8_t eight[12] =
{
		0x0e, 0x11, 0X11, 0X11, 0X11, 0X11, 0X0e, 0X11, 0X11, 0X11, 0X11, 0X0e
};
const uint8_t nine[12] =
{
		0x0e, 0x11, 0X01, 0X01, 0X01, 0X01, 0X0f, 0X11, 0X11, 0X11, 0X11, 0X0e
};
const uint8_t colon[12] =
{
		0x00, 0x00, 0X00, 0X03, 0X03, 0X00, 0X00, 0X00, 0X03, 0X03, 0X00, 0X00
};
const uint8_t am[12] =
{
		0x15, 0x15, 0X15, 0X1b, 0X11, 0X00, 0X11, 0X11, 0X1f, 0X11, 0X0a, 0X04
};
const uint8_t pm[12] =
{
		0x15, 0x15, 0X15, 0X1b, 0X11, 0X00, 0X10, 0X10, 0X1c, 0X12, 0X12, 0X1c
};


void setup()
{
	// Setup DCO Freq...used as input to timers A
	WDTCTL = WDTPW + WDTHOLD;                 				// Stop WDT
	BCSCTL1 |= DIVA_3 + 15;									// SET RSELx to15, Divide ACLK by 3
	BCSCTL3 |= XCAP_2;
	DCOCTL |= BIT5 + BIT6 + BIT7;							// SET DCOx to 7...should set DCO ~16Mhz--26MHz...no oscilloscope. :(

	// Timer A0 setup count up compare interrupts @ high speed for write data to array from DCO
	TA0CCTL0 = CCIE;                             			// Timer A0 CCR0 interrupt enabled
	TA0CTL = TASSEL_2 + MC_1 + ID_0;           				// SMCLK no division, upmode
	TA0CCR0 =  5000;                            				// Assuming DCO is running at 16MHz, throws interrupt every 312uS, approx 3000Hz, 16 rows to display update at < 180Hz

	TA1CCTL0 = CCIE;
	TA1CTL = TASSEL_1 + ID_3 + MC_1;						// TA1 on ACLK, Div by 8, upmode
	TA1CCR0 = 511;											// 32768/64 = 511 Throws interrupt @ 1Hz

	P1SEL = BIT2 + BIT4;
	P1SEL2 = BIT2 + BIT4;
	P2DIR = 0xFF;								              // Port B all output
	P2OUT &= 0x00;												// All pins port B 0

	// Setup SPI as master at SMCLK=DCO
	UCA0CTL0 |= UCCKPL + UCMST + UCSYNC;  					// 3-pin, 8-bit SPI master
	UCA0CTL1 |= UCSSEL_2;										// SMCLK as USI source
	//UCA0BR0 |= 0x02;                          // /2
	//UCA0BR1 = 0;                              //
	UCA0MCTL = 0;
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

	_BIS_SR(GIE);                   			 // Enter LPM0 w/ interrupt
}

void rowselect(int curr_row)
{
	curr_row = curr_row << 1;					// Bitwise shift left, as Bit0 is used as latch (probably bad practice)...
	P2OUT = curr_row;
}

void update_array()
{
	uint8_t i;
		volatile unsigned long tmp1;

		lowbyte = 0xFF;
		medlowbyte = 0xFF;
		medhighbyte = 0xFF;
		highbyte = 0xFF;

		// This for loop extracts the lowbytes...note that i is an int, numcolumns = 31, numcolumns/2 = 15 (int truncates to 15)
		for(i=0; i<4; i++)
		{
			tmp1 =  array[row];		// Bitwise and shift by i columns to extract bit from array in tmp

			switch(i)
			{
			case(0):
					lowbyte = lowbyte &= tmp1;
					break;
			case(1):
					tmp1 = tmp1 >> 8;
					medlowbyte = medlowbyte &= tmp1;
					break;
			case(2):
					tmp1 = tmp1 >> 16;
					medhighbyte = medhighbyte &= tmp1;
					break;
			case(3):
					tmp1 = tmp1 >> 24;
					highbyte = highbyte &= tmp1;
					break;
			}
		}

		// Now we select row and display data...

		P2OUT |= BIT5;								// sets output enable high to stop display of line when changing data

		UCA0TXBUF = lowbyte;
		while (!(IFG2 & UCA0TXIFG));						// Sends data out to STDP05 LED Drivers...

		UCA0TXBUF = medlowbyte;
		while (!(IFG2 & UCA0TXIFG));

		UCA0TXBUF = medhighbyte;
		while (!(IFG2 & UCA0TXIFG));

		UCA0TXBUF = highbyte;
		while (!(IFG2 & UCA0TXIFG));


		 rowselect(row);
		 if(row < NUMROWS)
		 	{row++;}
		 else
		 	{row = 0;}

		 P2OUT |= BIT0;								// Pulses pin P2.0 TO latch in data to stp08dp05
		 P2OUT &= ~BIT0;

		 P2OUT &= ~BIT5;							// Resets Bit5 low to turn line back on
}


void swap_array()
{
	if(swap_array_flag)
	{
		//memcpy(&nextarray, &array_nminus2, sizeof(array_nminus2));		// Swaps nextarray with nminus2
		memcpy(&array, &nextarray, sizeof(array));						// Swaps nextarray with array

		swap_array_flag = 0;
	}

}


void seed_array()
{
	uint8_t i;
	for(i=0; i<=NUMROWS; i++)
	{nextarray[i] = 0x00000000;}					// Clears nextarray

	srand(TA0R);									// Seeds random numbers from Timer 0.
	volatile long int x, y;								// y variable represents current column, x-->row
	for (y = 0; y <= NUMCOLUMNS; y++)			// Creates the random initial state of the array with 1s and 0s.
		{
			for (x = 0; x <= NUMROWS; x++)
			{
				if(rand() % 2 == 0)
				{
					array[x] |= 1L << y;			// Sets bit in array
				}
				else
				{
					array[x] &= ~(1L << y);		// resets bit in array
				}
			}
		}

}

void reset_game()
{
	P2OUT |= BIT5;								// Sets bit5 to clear array
	uint8_t i;

	for(i=0; i<=NUMROWS; i++)
	{
		array[i] = 0x00000000;
		nextarray[i] = 0x00000000;
		//update_array();
	}

	_delay_cycles(3000000);
	row = 0;

	//P2OUT &= ~BIT5								// Resets bit5 to turn array back on
	for(i=0; i<=NUMROWS; i++)
	{
		array[i] = 0xFFFFFFFF;
		update_array();
		_delay_cycles(500000);
	}
	_delay_cycles(500000);

	seed_array();
	reset_game_flag = 0;
}

uint8_t return_bit(unsigned long int bit)
{
	if (bit != 0)
		{return 1;}
	else
		return 0;
}

// Works with life subroutine to calculate the number of neighbors each cell has in the current array.  Note that this is modification of original code which used full array of uint8_t ints
// 32 x 16 = 512 byte for each array...too large to load program in RAM.  Reduced to array of uint32_t x 16 rows...now 64 byte each 8 fold reduction in memory space allocated but needs cumbersome
// bitwise operations to extract individual bits from the array of uint32_t
uint16_t calc_neighbors(uint8_t current_row, uint8_t current_column)
{
	volatile uint32_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	uint8_t num_of_neighbors;
	// Calc boundry conditions first...the four vertices of the array
	// Lower Right
	if(current_row == 0 && current_column == 0)							/// Problem within subroutine is causing array to be overwritten....
	{
		tmp1 = array[current_row];
		tmp1 = tmp1 &= 1L << (current_column +1);
		tmp1 = return_bit(tmp1);

		tmp2 = array[current_row];
		tmp2 = tmp2 &= 1L << (NUMCOLUMNS);
		tmp2 = return_bit(tmp2);

		tmp3 = array[current_row+1];
		tmp3 = tmp3 &= 1L << (NUMCOLUMNS);
		tmp3 = return_bit(tmp3);

		tmp4 = array[current_row+1];
		tmp4 = tmp4 &= 1L << (current_column+1);
		tmp4 = return_bit(tmp4);

		tmp5 = array[current_row+1];
		tmp5 = tmp5 &= 1L << (current_column);
		tmp5 = return_bit(tmp5);

		tmp6 = array[NUMROWS];
		tmp6 = tmp6 &= 1L << (current_column);
		tmp6 = return_bit(tmp6);

		tmp7 = array[NUMROWS];
		tmp7 = tmp7 &= 1L << (current_column+1);
		tmp7 = return_bit(tmp7);

		tmp8 = array[NUMROWS];
		tmp8 = tmp8 &= 1L << (NUMCOLUMNS);
		tmp8 = return_bit(tmp8);


		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	// Lower Left
	if(current_row == 0 && current_column == NUMCOLUMNS)
	{
		tmp1 = array[current_row];
		tmp1 = tmp1 &= 1L << (current_column -1);
		tmp1 = return_bit(tmp1);

		tmp2 = array[current_row+1];
		tmp2 = tmp2 &= 1L << (current_column);
		tmp2 = return_bit(tmp2);

		tmp3 = array[current_row+1];
		tmp3 = tmp3 &= 1L << (current_column-1);
		tmp3 = return_bit(tmp3);

		tmp4 = array[current_row];
		tmp4 = tmp4 &= 1L << (0);
		tmp4 = return_bit(tmp4);


		tmp5 = array[current_row+1];
		tmp5 = tmp5 &= 1L << (0);
		tmp5 = return_bit(tmp5);

		tmp6 = array[NUMROWS];
		tmp6 = tmp6 &= 1L << (current_column);
		tmp6 = return_bit(tmp6);


		tmp7 = array[NUMROWS];
		tmp7 = tmp7 &= 1L << (current_column-1);
		tmp7 = return_bit(tmp7);


		tmp8 = array[NUMROWS];
		tmp8 = tmp8 &= 1L << (0);
		tmp8 = return_bit(tmp8);


		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//upper right
	if(current_row == NUMROWS && current_column == 0)
	{
		tmp1 = array[current_row];
		tmp1 = tmp1 &= 1L << (current_column +1);
		tmp1 = return_bit(tmp1);

		tmp2 = array[current_row-1];
		tmp2 = tmp2 &= 1L << (current_column-1);
		tmp2 = return_bit(tmp2);

		tmp3 = array[current_row-1];
		tmp3 = tmp3 &= 1L << (current_column);
		tmp3 = return_bit(tmp3);

		tmp4 = array[current_row];
		tmp4 = tmp4 &= 1L << (NUMCOLUMNS);
		tmp4 = return_bit(tmp4);

		tmp5 = array[current_row-1];
		tmp5 = tmp5 &= 1L << (NUMCOLUMNS);
		tmp5 = return_bit(tmp5);

		tmp6 = array[0];
		tmp6 = tmp6 &= 1L << (current_column);
		tmp6 = return_bit(tmp6);

		tmp7 = array[0];
		tmp7 = tmp7 &= 1L << (current_column+1);
		tmp7 = return_bit(tmp7);

		tmp8 = array[0];
		tmp8 = tmp8 &= 1L << (NUMCOLUMNS);
		tmp8 = return_bit(tmp8);

		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//upper left
	if(current_row == NUMROWS && current_column == NUMCOLUMNS)
	{
		tmp1 = array[current_row];
		tmp1 = tmp1 &= 1L << (current_column -1);
		tmp1 = return_bit(tmp1);

		tmp2 = array[current_row-1];
		tmp2 = tmp2 &= 1L << (current_column-1);
		tmp2 = return_bit(tmp2);

		tmp3 = array[current_row-1];
		tmp3 = tmp3 &= 1L << (current_column);
		tmp3 = return_bit(tmp3);

		tmp4 = array[0];
		tmp4 = tmp4 &= 1L << (current_column);
		tmp4 = return_bit(tmp4);

		tmp5 = array[0];
		tmp5 = tmp5 &= 1L << (current_column-1);
		tmp5 = return_bit(tmp5);

		tmp6 = array[current_row];
		tmp6 = tmp6 &= 1L << (0);
		tmp6 = return_bit(tmp6);

		tmp7 = array[current_row-1];
		tmp7 = tmp7 &= 1L << (0);
		tmp7 = return_bit(tmp7);

		tmp8 = array[0];
		tmp8 = tmp8 &= 1L << (0);
		tmp8 = return_bit(tmp8);

		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}

	// Now fall through to the edge cases which are not at vertices..
	//Bottom edge
	if(current_row == 0)
	{
		tmp1 = array[current_row];
		tmp1 = tmp1 &= 1L << (current_column +1);
		tmp1 = return_bit(tmp1);

		tmp2 = array[current_row+1];
		tmp2 = tmp2 &= 1L << (current_column+1);
		tmp2 = return_bit(tmp2);

		tmp3 = array[current_row+1];
		tmp3 = tmp3 &= 1L << (current_column);
		tmp3 = return_bit(tmp3);

		tmp4 = array[current_row+1];
		tmp4 = tmp4 &= 1L << (current_column-1);
		tmp4 = return_bit(tmp4);

		tmp5 = array[current_row];
		tmp5 = tmp5 &= 1L << (current_column-1);
		tmp5 = return_bit(tmp5);

		tmp6 = array[NUMROWS];
		tmp6 = tmp6 &= 1L << (current_column+1);
		tmp6 = return_bit(tmp6);

		tmp7 = array[NUMROWS];
		tmp7 = tmp7 &= 1L << (current_column);
		tmp7 = return_bit(tmp7);

		tmp8 = array[NUMROWS];
		tmp8 = tmp8 &= 1L << (current_column-1);
		tmp8 = return_bit(tmp8);

		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//Top edge
	if(current_row == NUMROWS)
	{
		tmp1 = array[current_row];
		tmp1 = tmp1 &= 1L << (current_column +1);
		tmp1 = return_bit(tmp1);

		tmp2 = array[current_row-1];
		tmp2 = tmp2 &= 1L << (current_column+1);
		tmp2 = return_bit(tmp2);

		tmp3 = array[current_row-1];
		tmp3 = tmp3 &= 1L << (current_column);
		tmp3 = return_bit(tmp3);

		tmp4 = array[current_row-1];
		tmp4 = tmp4 &= 1L << (current_column-1);
		tmp4 = return_bit(tmp4);

		tmp5 = array[current_row];
		tmp5 = tmp5 &= 1L << (current_column-1);
		tmp5 = return_bit(tmp5);

		tmp6 = array[0];
		tmp6 = tmp6 &= 1L << (current_column+1);
		tmp6 = return_bit(tmp6);

		tmp7 = array[0];
		tmp7 = tmp7 &= 1L << (current_column);
		tmp7 = return_bit(tmp7);

		tmp8 = array[0];
		tmp8 = tmp8 &= 1L << (current_column-1);
		tmp8 = return_bit(tmp8);

		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//Right edge
	if(current_column == 0)
	{
		tmp1 = array[current_row+1];
		tmp1 = tmp1 &= 1L << (current_column);
		tmp1 = return_bit(tmp1);

		tmp2 = array[current_row+1];
		tmp2 = tmp2 &= 1L << (current_column+1);
		tmp2 = return_bit(tmp2);

		tmp3 = array[current_row];
		tmp3 = tmp3 &= 1L << (current_column+1);
		tmp3 = return_bit(tmp3);

		tmp4 = array[current_row-1];
		tmp4 = tmp4 &= 1L << (current_column+1);
		tmp4 = return_bit(tmp4);

		tmp5 = array[current_row-1];
		tmp5 = tmp5 &= 1L << (current_column);
		tmp5 = return_bit(tmp5);

		tmp6 = array[current_row+1];
		tmp6 = tmp6 &= 1L << (NUMCOLUMNS);
		tmp6 = return_bit(tmp6);

		tmp7 = array[current_row];
		tmp7 = tmp7 &= 1L << (NUMCOLUMNS);
		tmp7 = return_bit(tmp7);

		tmp8 = array[current_row-1];
		tmp8 = tmp8 &= 1L << (NUMCOLUMNS);
		tmp8 = return_bit(tmp8);

		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//Left edge
	if(current_column == NUMCOLUMNS)
	{
		tmp1 = array[current_row+1];
		tmp1 = tmp1 &= 1L << (current_column);
		tmp1 = return_bit(tmp1);

		tmp2 = array[current_row+1];
		tmp2 = tmp2 &= 1L << (current_column-1);
		tmp2 = return_bit(tmp2);

		tmp3 = array[current_row];
		tmp3 = tmp3 &= 1L << (current_column-1);
		tmp3 = return_bit(tmp3);

		tmp4 = array[current_row-1];
		tmp4 = tmp4 &= 1L << (current_column-1);
		tmp4 = return_bit(tmp4);

		tmp5 = array[current_row-1];
		tmp5 = tmp5 &= 1L << (current_column);
		tmp5 = return_bit(tmp5);

		tmp6 = array[current_row+1];
		tmp6 = tmp6 &= 1L << (0);
		tmp6 = return_bit(tmp6);

		tmp7 = array[current_row];
		tmp7 = tmp7 &= 1L << (0);
		tmp7 = return_bit(tmp7);

		tmp8 = array[current_row-1];
		tmp8 = tmp8 &= 1L << (0);
		tmp8 = return_bit(tmp8);

		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
				return num_of_neighbors;
	}

	// Fall through to base case in center of Board...
	tmp1 = array[current_row+1];
	tmp1 = tmp1 &= 1L << (current_column +1);
	tmp1 = return_bit(tmp1);

	tmp2 = array[current_row+1];
	tmp2 = tmp2 &= 1L << (current_column);
	tmp2 = return_bit(tmp2);

	tmp3 = array[current_row+1];
	tmp3 = tmp3 &= 1L << (current_column-1);
	tmp3 = return_bit(tmp3);

	tmp4 = array[current_row];
	tmp4 = tmp4 &= 1L << (current_column-1);
	tmp4 = return_bit(tmp4);

	tmp5 = array[current_row-1];
	tmp5 = tmp5 &= 1L << (current_column-1);
	tmp5 = return_bit(tmp5);

	tmp6 = array[current_row-1];
	tmp6 = tmp6 &= 1L << (current_column);
	tmp6 = return_bit(tmp6);

	tmp7 = array[current_row-1];
	tmp7 = tmp7 &= 1L << (current_column+1);
	tmp7 = return_bit(tmp7);

	tmp8 = array[current_row];
	tmp8 = tmp8 &= 1L << (current_column+1);
	tmp8 = return_bit(tmp8);

	num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
		return num_of_neighbors;
}

void life()
{
	volatile unsigned long int test;
	uint16_t num_neighbors;
	volatile unsigned long int x, y;

	if (reset_game_flag == 1)
		{
			reset_game();
		}

	if (swap_array_flag)
		{
			return;						// If swap array set, do not recalculate array!
		}


	for (y = 0; y <= NUMCOLUMNS; y++)
		{
			for (x = 0; x <= NUMROWS; x++)
			{
				num_neighbors = calc_neighbors(x,y);


				test = array[x];
				// Current array live cell cases-----------------------------------
				if ( (test &= 1L << y) != 0)
				{
					if(num_neighbors < 2)
					{
						nextarray[x] &= ~(1L << y);				// Sets nextarray bit to dead state
					}
					if(num_neighbors == 2 || num_neighbors == 3)
					{
						nextarray[x] |= (1L << y);				// Sets nextarray bit to live state
					}
					if(num_neighbors > 3)
					{
						nextarray[x] &= ~(1L << y);				// Sets nextarray bit to dead state
					}
				}
				// Current array dead cell cases-----------------------------------
				else
				{
					if(num_neighbors == 3)
					{
						nextarray[x] |= (1L << y);				// Sets nextarray bit to live state
					}
				}
			}
		}

		// Restarts if the array is frozen or has still life with period 2
		if ( memcmp(&array, &nextarray, sizeof (array)) == 0 )		// Uses memcmp to determine if the arrays are stuck in identical state
		{														// to reset game
			reset_game_flag = 1;
		}


		swap_array_flag = 1; 									// Sets swap array flag to allow array to change at next 1sec interval


}

//======================================================================================================================================================
/*	Update clock function updates the array to hold the proper pattern for the current time.  Pass in digit to determine which portion of array
 * 	to be currently updated by function.  digit == 0, ones_mins.  digit == 1, tens_mins.  digit == 2, ones_hours.  digit == 3, tens_hours.
 */
//======================================================================================================================================================
void update_clock(uint8_t digit)
{
	uint8_t i, shift = 8, num = ones_mins;
	uint32_t tmp;

	if(digit == 3)
	{
		shift = 29;
		num = tens_hrs;
	}
	else if(digit == 2)
	{
		shift = 23;
		num = ones_hrs;
	}
	else if(digit == 1)
	{
		shift = 14;
		num = tens_mins;
	}

	/*	We want to now update entire array with the correct time...get individual time characters and load them to the array line by line in for loop */
	switch(num)
	{
		case(0):
			if(digit == 3)
			{
				for(i=2; i<14; i++)
				{
					tmp = 0x00000000;
					tmp = tmp << shift;
					array[i] |= tmp;				// So that we don't see the zero in front of the one prior to 10 o'clock
				}
				break;
			}
			for(i=2; i<14; i++)
			{
				tmp = zero[i-2];
				tmp = tmp << shift;
				array[i] |= tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;

		case(1):
			for(i=2; i<14; i++)
			{
				tmp = one[i-2];
				tmp = tmp << shift;
				array[i] |= tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;

		case(2):
			for(i=2; i<14; i++)
			{
				tmp = two[i-2];
				tmp = tmp << shift;
				array[i] |=  tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;

		case(3):
			for(i=2; i<14; i++)
			{
				tmp = three[i-2];
				tmp = tmp << shift;
				array[i] |=  tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;

		case(4):
			for(i=2; i<14; i++)
			{
				tmp = four[i-2];
				tmp = tmp << shift;
				array[i] |=  tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;

		case(5):
			for(i=2; i<14; i++)
			{
				tmp = five[i-2];
				tmp = tmp << shift;
				array[i] |=  tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;

		case(6):
			for(i=2; i<14; i++)
			{
				tmp = six[i-2];
				tmp = tmp << shift;
				array[i] |=  tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;

		case(7):
			for(i=2; i<14; i++)
			{
				tmp = seven[i-2];
				tmp = tmp << shift;
				array[i] |=  tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;

		case(8):
			for(i=2; i<14; i++)
			{
				tmp = eight[i-2];
				tmp = tmp << shift;
				array[i] |=  tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;

		case(9):
			for(i=2; i<14; i++)
			{
				tmp = nine[i-2];
				tmp = tmp << shift;
				array[i] |=  tmp;				// Remember that numeric array is smaller than size of array...may cause bug...numeric array 8bits wide; we're ORing a section smaller than this
			}
			break;
	}
/*
	switch(character)
	{
		case(0):
			for(i=0; i<12; i++)
			{
				array[i+2] = zero[i];
			}
			break;
		case(1):
			for(i=0; i<12; i++)
			{
				array[i+2] = one[i];
			}
			break;
		case(2):
			for(i=0; i<12; i++)
			{
				array[i+2] = two[i] << 4;
			}
			break;
		case(3):
			for(i=0; i<12; i++)
			{
				array[i+2] = three[i];
			}
			break;
		case(4):
			for(i=0; i<12; i++)
			{
				array[i+2] = four[i];
			}
			break;
		case(5):
			for(i=0; i<12; i++)
			{
				array[i+2] = five[i];
			}
			break;
		case(6):
			for(i=0; i<12; i++)
			{
				array[i+2] = six[i];
			}
			break;
		case(7):
			for(i=0; i<12; i++)
			{
				array[i+2] = seven[i];
			}
			break;
		case(8):
			for(i=0; i<12; i++)
			{
				array[i+2] = eight[i];
			}
			break;
		case(9):
			for(i=0; i<12; i++)
			{
				array[i+2] = nine[i];
			}
			break;
	}
*/

}
void update_time()
{
// Catches case of 59 min rollover to next hour.
	if(ones_mins == 9 && tens_mins == 5)
	{
		ones_mins = 0;
		tens_mins = 0;

		// Case of 12th hr rollover to 1
		if (tens_hrs == 1 && ones_hrs == 2 )
		{
			tens_hrs = 0;
			ones_hrs = 1;
			return;
		}
		if (tens_hrs == 1 && ones_hrs == 1)
		{
			am_pm ^= 1;
			ones_hrs++;
			return;
		}								// Toggles from am to pm
		if (ones_hrs == 9)
		{
			tens_hrs = 1;
			ones_hrs = 0;
			return;
		}

		ones_hrs++;
	}

	else if(ones_mins == 9)
	{
		ones_mins = 0;
		tens_mins++;
	}

	else				// Falls through to just increment minutes by 1
	{
		ones_mins++;
		return;
	}
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
/*	one_sec_flag++;
		if(one_sec_flag >= 500)
		{
			one_sec_flag = 0;
			swap_array();
		}*/

		update_array();


}


// Timer A0 interrupt service routine.  This implements the clock function of the LED array.
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A1 (void)
{
/*	count++;

	if(count >= 60)
	{
		count = 0;
		reset_game();
	}
*/
	count++;
	colon_blink ^= 1;							// Toggles so we get colon to blink at 2 Hz

/*	if(count >= 60)
		{
			count = 0;
			update_time();
		}
*/
	update_time();

	/*
update_clock(num);
num++;
	if(num >= 10)
		{num = 0;} */

	uint8_t i;
	uint32_t tmp;
	for(i=0; i<16; i++)
		{
			array[i] = 0x00000000;		// First, clears array before we update pattern for new time.  We can improve update speed if we only update what we need, but this may be fast enough
		}

	for(i=0; i<4; i++)
	{
		update_clock(i);				// Updates all 4 digits of array.
	}

	if(colon_blink == 1)
	{
		for(i=2; i<14; i++)
		{
			tmp = colon[i-2];
			tmp = tmp << 20;
			array[i] |=  tmp;			// colonblink outside update_clock so we're not wasting cyles to constantly update this.
		}
	}

	if(am_pm == 1)
	{
		for(i=2; i<14; i++)
		{
			tmp = pm[i-2];
			tmp = tmp << 2;
			array[i] |=  tmp;			// colonblink outside update_clock so we're not wasting cyles to constantly update this.
		}
	}

	for(i=2; i<14; i++)
		{
			tmp = am[i-2];
			tmp = tmp << 2;
			array[i] |=  tmp;			// colonblink outside update_clock so we're not wasting cyles to constantly update this.
		}
}
