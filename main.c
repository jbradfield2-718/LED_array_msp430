#include "msp430g2533.h"
#include "stdint.h"
#include "main_functions.h"

#define MODE	(P1IN &= 1)							// Gets bit 0 to determine mode


void main(void)
{
	setup();
	if(MODE == 1)
	{seed_array();}

	for(;;)
	{
		life();
	}

  }

