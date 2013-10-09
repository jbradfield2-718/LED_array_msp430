#include <msp430g2231.h>
#include "stdint.h"
#include "main_functions.h"

extern volatile uint8_t byte;

void main(void)
{
	setup();

	for(;;){}

  }                                  //Loop forever, we work with interrupts!


