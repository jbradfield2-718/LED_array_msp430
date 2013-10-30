//#include <msp430.h>
#include "msp430g2533.h"
#include "stdint.h"


extern void setup();
extern void seed_array();
extern void life();

#ifndef MAIN_FUNCTIONS_H_
#define MAIN_FUNCTIONS_H_

#define setOutput(ddr, pin) ((ddr) |= (1 << (pin)))
#define setLow(port, pin) ((port) &= ~(1 << (pin)))
#define setHigh(port, pin) ((port) |= (1 << (pin)))
#define pulse(port, pin) do { \
                         	setHigh((port), (pin)); \
                         	setLow((port), (pin)); \
                         } while (0)
#define outputState(port, pin) ((port) & (1 << (pin)))

#ifndef A0
#define A0		BIT1
#endif

#ifndef A1
#define A1		BIT2
#endif

#ifndef	A3
#define A3		BIT3
#endif

#ifndef	LATCH
#define LATCH	BIT0
#endif

#ifndef	CLEAR
#define CLEAR	BIT5
#endif

#ifndef	NUMROWS
#define NUMROWS	15			// REMEMBER, WE HAVE A ROW 0 (TOTAL ROWS 0-15)
#endif

#ifndef	NUMCOLUMNS
#define NUMCOLUMNS	31			// REMEMBER, WE HAVE A COLUMN 0 (TOTAL COLUMNS 0-31)
#endif


#endif /* MAIN_FUNCTIONS_H_ */
