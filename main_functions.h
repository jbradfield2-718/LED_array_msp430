
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

#ifndef DATA
#define DATA	PC0
#endif

#ifndef LATCH
#define LATCH	PC1
#endif

#ifndef	CLOCK
#define CLOCK	PC2
#endif

#ifndef	OUTEN
#define OUTEN	PC3
#endif

#ifndef  STDP05_N
#define STDP05_N	1
#endif


#endif /* MAIN_FUNCTIONS_H_ */
