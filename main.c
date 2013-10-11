#include <msp430g2231.h>
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


