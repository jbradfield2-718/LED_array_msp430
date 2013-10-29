//#include <msp430.h>
#include "msp430g2533.h"
#include "stdint.h"
#include "main_functions.h"


void main(void)
{
	setup();
	seed_array();

	for(;;)
	{
		life();
	}

  }

