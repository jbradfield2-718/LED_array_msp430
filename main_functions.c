/*
 * main_functions.c
 *
 * Created: 9/2/2013 3:03:49 PM
 *  Author: Turing
 */

#include <msp430g2231.h>
#include "stdint.h"
#include "main_functions.h"


volatile int one_sec_flag = 0;
volatile int swap_array_flag = 0;
volatile uint8_t reset_game_flag = 0;
volatile uint8_t row = 0;
volatile uint16_t highbytes = 0xab03;
volatile uint16_t lowbytes = 0xffff;
volatile uint16_t highbytes_last = 0xaaaa;
volatile uint16_t lowbytes_last = 0xaaaa;
//volatile uint8_t array[NUMROWS][NUMCOLUMNS];
//volatile uint8_t nextarray[NUMROWS][NUMCOLUMNS];

volatile uint32_t array[NUMROWS +1] =
{
		0x00000000, 0x00000000, 0x00000000, 0x00000000,  \
		0x00000000, 0x00000000, 0x00000000, 0x00000000,  \
		0x00000000, 0x00000000, 0x00000000, 0x00000000,  \
		0x00000000, 0x00000000, 0x00000000, 0x00000000

};

volatile uint32_t nextarray[NUMROWS +1] =
{
		0x00000000, 0x00000000, 0x00000000, 0x00000000,  \
		0x00000000, 0x00000000, 0x00000000, 0x00000000,  \
		0x00000000, 0x00000000, 0x00000000, 0x00000000,  \
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


void swap_array()
{
	if(swap_array_flag == 0)
	{
		highbytes = highbytes_last;
		lowbytes = lowbytes_last;
		swap_array_flag = 1;
	}

	else
	{
		highbytes = 0xab03;
		lowbytes = 0xffff;
		swap_array_flag = 0;
	}
}


void seed_array()
{
	rand(TAR);									// Seeds random numbers from Timer 0.
	uint8_t x, y;								// y variable represents current column, x-->row
	for (y = 0; y <= NUMCOLUMNS; y++)			// Creates the random initial state of the array with 1s and 0s.
		{
			for (x = 0; x <= NUMROWS; x++)
			{
				if(rand() % 2 == 0)
				{
					array[x][y] = 1;
				}
				else
				{
					array[x][y] = 0;
				}
			}
		}

}

void array_setup()
{
	row = 0; uint8_t column = 0;
		for (column = 0; column < NUMCOLUMNS; column++)
		{
			for (row = 0; row < NUMROWS; row++)
			{
				nextarray[row][column] = 0;
			}
		}
		row = 0; column = 0;

		seed_array();
	}

void reset_game()
{
	_delay_cycles(8000000);
	seed_array();
	reset_game_flag = 0;
}

// Works with life subroutine to calculate the number of neighbors each cell has in the current array
uint16_t calc_neighbors(uint8_t current_row, uint8_t current_column)
{
	// Calc boundry conditions first...the four vertices of the array
	// Lower Right
	if(current_row == 0 && current_column == 0)
	{
		return array[current_row][current_column +1] + array[current_row][NUMCOLUMNS-1] + array[current_row+1][NUMCOLUMNS-1] + array[current_row+1][current_column+1] +
				array[current_row+1][current_column] + array[NUMROWS-1][current_column] + array[NUMROWS-1][current_column+1] + array[NUMROWS-1][NUMCOLUMNS-1];
	}
	// Lower Left
	if(current_row == 0 && current_column == NUMCOLUMNS-1)
	{
		return array[current_row][current_column -1] + array[current_row+1][current_column] + array[current_row+1][current_column-1] + array[current_row][0] +
				array[current_row+1][0] + array[NUMROWS-1][current_column] + array[NUMROWS-1][current_column-1] + array[NUMROWS-1][0];
	}
	//upper right
	if(current_row == NUMROWS-1 && current_column == 0)
	{
		return array[current_row][current_column +1] + array[current_row-1][current_column-1] + array[current_row-1][current_column] + array[current_row][NUMCOLUMNS-1] +
				array[current_row-1][NUMCOLUMNS-1] + array[0][current_column] + array[0][current_column+1] + array[0][NUMCOLUMNS-1];
	}
	//upper left
	if(current_row == NUMROWS-1 && current_column == NUMCOLUMNS-1)
	{
		return array[current_row][current_column -1] + array[current_row-1][current_column-1] + array[current_row-1][current_column] + array[0][current_column] +
				array[0][current_column-1] + array[current_row][0] + array[current_row-1][0] + array[0][0];
	}

	// Now fall through to the edges not at vertices..
	//Bottom edge
	if(current_row == 0)
	{
		return array[current_row][current_column +1] + array[current_row+1][current_column+1] + array[current_row+1][current_column] + array[current_row+1][current_column-1] +
				array[current_row][current_column-1] + array[NUMROWS-1][current_column+1] + array[NUMROWS-1][current_column] + array[NUMROWS-1][current_column-1];
	}
	//Top edge
	if(current_row == NUMROWS-1)
	{
		return array[current_row][current_column +1] + array[current_row-1][current_column+1] + array[current_row-1][current_column] + array[current_row-1][current_column-1] +
				array[current_row][current_column-1] + array[0][current_column+1] + array[0][current_column] + array[0][current_column-1];
	}
	//Right edge
	if(current_column == 0)
	{
		return array[current_row+1][current_column] + array[current_row+1][current_column+1] + array[current_row][current_column+1] + array[current_row-1][current_column+1] +
				array[current_row-1][current_column] + array[current_row+1][NUMCOLUMNS-1] + array[current_row][NUMCOLUMNS-1] + array[current_row-1][NUMCOLUMNS-1];
	}
	//Left edge
	if(current_column == NUMCOLUMNS-1)
	{
		return array[current_row+1][current_column] + array[current_row+1][current_column-1] + array[current_row][current_column-1] + array[current_row-1][current_column-1] +
				array[current_row-1][current_column] + array[current_row+1][0] + array[current_row][0] + array[current_row-1][0];
	}

	// Fall through to base case in center of Board...
	return array[current_row+1][current_column +1] + array[current_row+1][current_column] + array[current_row+1][current_column-1] + array[current_row][current_column-1] +
				array[current_row-1][current_column-1] + array[current_row-1][current_column] + array[current_row-1][current_column+1] + array[current_row][current_column+1];
}

void life()
{
	uint16_t num_neighbors;
	unsigned int x, y;
	if (reset_game_flag == 1)
		{
			reset_game();
		}

	for (y = 0; y <= NUMROWS; y++)
		{
			for (x = 0; x <= NUMROWS; x++)
			{
				num_neighbors = calc_neighbors(x,y);

				// Current array live cell cases-----------------------------------
				if (array[x][y] == 1)
				{
					if(num_neighbors < 2)
					{
						nextarray[x][y] = 0;
					}
					if(num_neighbors == 2 || num_neighbors == 3)
					{
						nextarray[x][y] = 1;
					}
					if(num_neighbors > 3)
					{
						nextarray[x][y] = 0;
					}
				}
				// Current array dead cell cases-----------------------------------
				else
				{
					if(num_neighbors == 3)
					{
						nextarray[x][y] = 1;
					}
				}
			}
		}

		// Restarts if the array is frozen
		if (memcmp(array, nextarray, sizeof (array)) == 0)
		{
			reset_game_flag = 1;

		}



}

void update_array()
{
	life();

}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	one_sec_flag++;
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
	uint8_t i;

	/*
	// This for loop extracts the lowbytes...note that i is an int, numcolumns = 31, numcolumns/2 = 15 (int truncates to 15)
	for(i=0; i<=NUMCOLUMNS/2; i++)
	{
		lowbytes |= array[row][i] << i;
	}
	// This for loop extracts the highbytes...note that i is an int, numcolumns = 31, numcolumns/2 = 15 (int truncates to 15)
	for(i=0; i<=NUMCOLUMNS/2; i++)
	{
		highbytes |= array[row][i] << i;
	}
	*/

	USISR = lowbytes;						// LOADS 16bits
	USICNT |= USI16B + 16;					// SENDS 16bits
	while ( ! (USIIFG & USICTL1) );			// Waits until lowbyte shifted in

	USISR = highbytes;						// LOADS
	USICNT |= USI16B + 17;					// SENDS.  Needs the extra bit or LED pattern is incorrect, off by 1!
	while ( ! (USIIFG & USICTL1) );			// Waits until highbyte shifted in

	 P1OUT |= BIT0;								// Pulses pin P1.0 TO latch in data to stp08dp05
	 P1OUT &= ~BIT0;
	 USICTL1 &= ~USIIFG;


}


