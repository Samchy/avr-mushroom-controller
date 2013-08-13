#include "timer0.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t OneSecondFlag = 0;

/**-------------------------------------------------------------------------------------------------
  Description : Set-up overflow interrupt on Timer0
-------------------------------------------------------------------------------------------------**/
void initTimer0()
{
	// Normal port operation
	TCCR0A = 0;
	// Prescaler 1
	TCCR0B = (1<<CS00);
	// Overflow interrupt
	TIMSK0 = (1<<TOIE0);
}

/**-------------------------------------------------------------------------------------------------
  Description : set OneSecondFlag to 1 each second
-------------------------------------------------------------------------------------------------**/
ISR (TIMER0_OVF_vect)
{
	static uint16_t count = 62500;// - 1s
	if(!--count)
	{
		OneSecondFlag = 1;
		count = 62500;
	}
}
