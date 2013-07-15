/*______________________________________________________________________
	Time measuring and delaying routines using 16-bit Timer1 

Controller:	ATmega 168 (Clock: 14,7456 Mhz-external)
Compiler:	AVR-GCC
Author:		Darius Berghe
Date:		Mar 2012

Description: 
	util/delay.h provides _delay_ms() and _delay_us() functions but 
	they require compile-time known argument so I wrote this piece 
	of code which overcomes this.
	
	The assumptions for these functions to work as expected are:
	
	- Timer1 counts from BOTTOM to MAX and resets back to BOTTOM
	- the argument given to basic_delay_ms or delay_us should not
	  surpass the maximum possible delay with a certain prescaler.
	
	Therefore, using a e.g. PWM with Timer1 will mess up everything. 
_________________________________________________________________________
*/

#ifndef _TIMER1_H
	#define _TIMER1_H
	
#include <stdint.h>
#include "Main.h"
#include "utils.h"

#define T1_PRESCALER 	8
#define T1_OVF_VALUE     65535

typedef uint16_t time_t ;


/**
__________________________________________
		  Function Prototypes 
**/
/* Initialize Timer1 Registers */
void initTimer1(void);

/*	Convert clock ticks to miliseconds  */
float tk2ms(time_t);

/*	Convert clock ticks to microseconds  */
float tk2us(time_t);

/*	Convert miliseconds to clock ticks */
time_t ms2tk(float);

/*	Convert microseconds to clock ticks */
time_t us2tk(float);


/*	Get current timer/counter value */
time_t clock(void);

/*	Get time difference in ticks */
time_t difftime_tk(time_t time2, time_t time1);

/*	Get time difference in microseconds */
float difftime_us(time_t time2, time_t time1);

/*	Get time difference in miliseconds */
float difftime_ms(time_t time2, time_t time1);


/* Busy delay in microseconds */
void delay_us(float);

/* Busy delay in miliseconds */
void basic_delay_ms(float);

/* Long delay in miliseconds */
void delay_ms(float);

#endif //__TIMER_H

