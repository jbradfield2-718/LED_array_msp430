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


volatile int one_sec_flag = 0;
volatile int swap_array_flag = 0;
volatile uint8_t reset_game_flag = 0;
volatile uint8_t row = 0;

volatile uint8_t lowbyte = 0x00;
volatile uint8_t medlowbyte = 0x00;
volatile uint8_t medhighbyte = 0x00;
volatile uint8_t highbyte = 0x00;

volatile uint32_t array[NUMROWS +1] =
{
		0xaaaaaaaa, 0xaaaaaaaa, 0x00000000, 0x00000000,
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

void setup()
{
	// Setup DCO Freq...used as input to timers A and B
	WDTCTL = WDTPW + WDTHOLD;                 				// Stop WDT
	BCSCTL1 |= 15;											// SET RSELx to15
	DCOCTL |= BIT5 + BIT6;									// SET DCOx to 3...should set DCO ~16Mhz...no oscilloscope. :(

	// Timer A setup count up compare interrupts @ high speed for write data to array
	TACCTL0 = CCIE;                             			// Timer A CCR0 interrupt enabled
	TACTL = TASSEL_2 + MC_1 + ID_0;           				// SMCLK no division, upmode
	TACCR0 =  5000;                            				// Assuming DCO is running at 16MHz, throws interrupt every 312uS, approx 3000Hz, 16 rows to display update at < 180Hz

	P1SEL = BIT2 + BIT4;
	P1SEL2 = BIT2 + BIT4;
	P2DIR = 0xFF;								              // Port B all output
	P2OUT &= 0x00;

	// Setup SPI as master at SMCLK=DCO
	UCA0CTL0 |= UCCKPL + UCMST + UCSYNC;  					// 3-pin, 8-bit SPI master
	UCA0CTL1 |= UCSSEL_2;										// SMCLK as USI source
	UCA0BR0 |= 0x02;                          // /2
	UCA0BR1 = 0;                              //
	UCA0MCTL = 0;
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

	_BIS_SR(GIE);                   			 // Enter LPM0 w/ interrupt
}

void rowselect(int curr_row)
{
	curr_row = curr_row << 1;					// Bitwise shift left, as Bit0 is used as latch (probably bad practice)...
	P2OUT = curr_row;
}


void swap_array()
{
	if(swap_array_flag)
	{
		memcpy(&array, &nextarray, sizeof(array));		// Swaps nextarray with array
		swap_array_flag = 0;
	}

}


void seed_array()
{
	srand(TAR);									// Seeds random numbers from Timer 0.
	uint8_t x, y;								// y variable represents current column, x-->row
	for (y = 0; y <= NUMCOLUMNS; y++)			// Creates the random initial state of the array with 1s and 0s.
		{
			for (x = 0; x <= NUMROWS; x++)
			{
				if(rand() % 2 == 0)
				{
					array[x] |= 1 << y;			// Sets bit in array
				}
				else
				{
					array[x] &= ~(1 << y);		// resets bit in array
				}
			}
		}

}

void reset_game()
{
	_delay_cycles(8000000);
	seed_array();
	reset_game_flag = 0;
}

// Works with life subroutine to calculate the number of neighbors each cell has in the current array.  Note that this is modification of original code which used full array of uint8_t ints
// 32 x 16 = 512 byte for each array...too large to load program in RAM.  Reduced to array of uint32_t x 16 rows...now 64 byte each 8 fold reduction in memory space allocated but needs cumbersome
// bitwise operations to extract individual bits from the array of uint32_t
uint16_t calc_neighbors(uint8_t current_row, uint8_t current_column)
{
	uint8_t num_of_neighbors, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
	// Calc boundry conditions first...the four vertices of the array
	// Lower Right
	if(current_row == 0 && current_column == 0)
	{
		tmp1 = array[current_row] &= 1L << (current_column +1);
		tmp2 = array[current_row] &= 1L << (NUMCOLUMNS-1);
		tmp3 = array[current_row+1] &= 1L << (NUMCOLUMNS-1);
		tmp4 = array[current_row+1] &= 1L << (current_column+1);
		tmp5 = array[current_row+1] &= 1L << (current_column);
		tmp6 = array[NUMROWS-1] &= 1L << (current_column);
		tmp7 = array[NUMROWS-1] &= 1L << (current_column+1);
		tmp8 = array[NUMROWS-1] &= 1L << (NUMCOLUMNS-1);
		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	// Lower Left
	if(current_row == 0 && current_column == NUMCOLUMNS-1)
	{
		tmp1 = array[current_row] &= 1L << (current_column -1);
		tmp2 = array[current_row+1] &= 1L << (current_column);
		tmp3 = array[current_row+1] &= 1L << (current_column-1);
		tmp4 = array[current_row] &= 1L << (0);
		tmp5 = array[current_row+1] &= 1L << (0);
		tmp6 = array[NUMROWS-1] &= 1L << (current_column);
		tmp7 = array[NUMROWS-1] &= 1L << (current_column-1);
		tmp8 = array[NUMROWS-1] &= 1L << (0);
		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//upper right
	if(current_row == NUMROWS-1 && current_column == 0)
	{
		tmp1 = array[current_row] &= 1L << (current_column +1);
		tmp2 = array[current_row-1] &= 1L << (current_column-1);
		tmp3 = array[current_row-1] &= 1L << (current_column);
		tmp4 = array[current_row] &= 1L << (NUMCOLUMNS-1);
		tmp5 = array[current_row-1] &= 1L << (NUMCOLUMNS-1);
		tmp6 = array[0] &= 1L << (current_column);
		tmp7 = array[0] &= 1L << (current_column+1);
		tmp8 = array[0] &= 1L << (NUMCOLUMNS-1);
		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//upper left
	if(current_row == NUMROWS-1 && current_column == NUMCOLUMNS-1)
	{
		tmp1 = array[current_row] &= 1L << (current_column -1);
		tmp2 = array[current_row-1] &= 1L << (current_column-1);
		tmp3 = array[current_row-1] &= 1L << (current_column);
		tmp4 = array[0] &= 1L << (current_column);
		tmp5 = array[0] &= 1L << (current_column-1);
		tmp6 = array[current_row] &= 1L << (0);
		tmp7 = array[current_row-1] &= 1L << (0);
		tmp8 = array[0] &= 1L << (0);
		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}

	// Now fall through to the edge cases which are not at vertices..
	//Bottom edge
	if(current_row == 0)
	{
		tmp1 = array[current_row] &= 1L << (current_column +1);
		tmp2 = array[current_row+1] &= 1L << (current_column+1);
		tmp3 = array[current_row+1] &= 1L << (current_column);
		tmp4 = array[current_row+1] &= 1L << (current_column-1);
		tmp5 = array[current_row] &= 1L << (current_column-1);
		tmp6 = array[NUMROWS-1] &= 1L << (current_column+1);
		tmp7 = array[NUMROWS-1] &= 1L << (current_column);
		tmp8 = array[NUMROWS-1] &= 1L << (current_column-1);
		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//Top edge
	if(current_row == NUMROWS-1)
	{
		tmp1 = array[current_row] &= 1L << (current_column +1);
		tmp2 = array[current_row-1] &= 1L << (current_column+1);
		tmp3 = array[current_row-1] &= 1L << (current_column);
		tmp4 = array[current_row-1] &= 1L << (current_column-1);
		tmp5 = array[current_row] &= 1L << (current_column-1);
		tmp6 = array[0] &= 1L << (current_column+1);
		tmp7 = array[0] &= 1L << (current_column);
		tmp8 = array[0] &= 1L << (current_column-1);
		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//Right edge
	if(current_column == 0)
	{
		tmp1 = array[current_row+1] &= 1L << (current_column);
		tmp2 = array[current_row+1] &= 1L << (current_column+1);
		tmp3 = array[current_row] &= 1L << (current_column+1);
		tmp4 = array[current_row-1] &= 1L << (current_column+1);
		tmp5 = array[current_row-1] &= 1L << (current_column);
		tmp6 = array[current_row+1] &= 1L << (NUMCOLUMNS-1);
		tmp7 = array[current_row] &= 1L << (NUMCOLUMNS-1);
		tmp8 = array[current_row-1] &= 1L << (NUMCOLUMNS-1);
		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
			return num_of_neighbors;
	}
	//Left edge
	if(current_column == NUMCOLUMNS-1)
	{
		tmp1 = array[current_row+1] &= 1L << (current_column);
		tmp2 = array[current_row+1] &= 1L << (current_column-1);
		tmp3 = array[current_row] &= 1L << (current_column-1);
		tmp4 = array[current_row-1] &= 1L << (current_column-1);
		tmp5 = array[current_row-1] &= 1L << (current_column);
		tmp6 = array[current_row+1] &= 1L << (0);
		tmp7 = array[current_row] &= 1L << (0);
		tmp8 = array[current_row-1] &= 1L << (0);
		num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
				return num_of_neighbors;
	}

	// Fall through to base case in center of Board...
	tmp1 = array[current_row+1] &= 1L << (current_column +1);
	tmp2 = array[current_row+1] &= 1L << (current_column);
	tmp3 = array[current_row+1] &= 1L << (current_column-1);
	tmp4 = array[current_row] &= 1L << (current_column-1);
	tmp5 = array[current_row-1] &= 1L << (current_column-1);
	tmp6 = array[current_row-1] &= 1L << (current_column);
	tmp7 = array[current_row-1] &= 1L << (current_column+1);
	tmp8 = array[current_row] &= 1L << (current_column+1);
	num_of_neighbors = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
		return num_of_neighbors;
}

void life()
{
	uint16_t num_neighbors;
	uint8_t x, y;

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

				// Current array live cell cases-----------------------------------
				if ( (array[y] &= 1 << x) == 1)
				{
					if(num_neighbors < 2)
					{
						nextarray[y] &= ~(1 << x);				// Sets nextarray bit to dead state
					}
					if(num_neighbors == 2 || num_neighbors == 3)
					{
						nextarray[y] |= (1 << x);				// Sets nextarray bit to live state
					}
					if(num_neighbors > 3)
					{
						nextarray[y] &= ~(1 << x);				// Sets nextarray bit to dead state
					}
				}
				// Current array dead cell cases-----------------------------------
				else
				{
					if(num_neighbors == 3)
					{
						nextarray[y] |= (1 << x);				// Sets nextarray bit to live state
					}
				}
			}
		}

		// Restarts if the array is frozen
	/*	if (memcmp(array, nextarray, sizeof (array)) == 0)		// Uses memcmp to determine if the arrays are stuck in identical state
		{														// to reset game
			reset_game_flag = 1;
		}
*/
		swap_array_flag = 1; 									// Sets swap array flag to allow array to change at next 1sec interval


}

void update_array()
{
	life();
}


// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
/*	one_sec_flag++;
		if(one_sec_flag >= 3200)
		{
			one_sec_flag = 0;
			swap_array();
		}

	rowselect(row);
	if(row <= NUMROWS)
	{row++;}
	else
	{row = 0;}

*/
	rowselect(5);						// Testing Only...
	uint8_t i, tmp2, tmp3;
	uint32_t tmp1;



	// This for loop extracts the lowbytes...note that i is an int, numcolumns = 31, numcolumns/2 = 15 (int truncates to 15)
	for(i=0; i<=NUMCOLUMNS; i++)
	{
		tmp1 =  array[row];		// Bitwise and shift by i columns to extract bit from array in tmp
		tmp2 = 1 << i;
		tmp3 = tmp1 & tmp2;

		if(i<4)
			{lowbyte |= tmp3;}				// Bitwise OR to load it into char to SPI
		else if(i>=4 && i<8)
			{tmp3 = tmp3 >> 4;				// Bitwise shift right to
			medlowbyte |= tmp3;}
		else if(i>=8 && i<12)
			{tmp3 = tmp3 >> 8;
			medhighbyte |= tmp3;}
		else if(i>=12 && i<16)
			{tmp3 = tmp3 >> 12;
			highbyte |= tmp3;}

		tmp1 = 0; tmp2 = 0; tmp3 = 0;							// clears temp variable to prevent possible overwrite errors.
	}

	UCA0TXBUF = lowbyte;
	while (!(IFG2 & UCA0TXIFG));						// Waits until lowbyte shifted in

	UCA0TXBUF = medlowbyte;
	while (!(IFG2 & UCA0TXIFG));

	UCA0TXBUF = medhighbyte;
	while (!(IFG2 & UCA0TXIFG));

	UCA0TXBUF = highbyte;
	while (!(IFG2 & UCA0TXIFG));

	 P2OUT |= BIT0;								// Pulses pin P2.0 TO latch in data to stp08dp05
	 P2OUT &= ~BIT0;


}


