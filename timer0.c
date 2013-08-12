#include "timer0.h"

uint8_t OneSecondFlag = 0;

void initTimer0()
{
	// Normal port operation
	TCCR0A = 0;
	// Prescaler 1
	TCCR0B = (1<<CS00);
	// Overflow interrupt
	TIMSK0 = (1<<TOIE0);
}

ISR (TIMER0_OVF_vect)
{
	static uint16_t count = 62500;
	if(!--count)
	{
		OneSecondFlag = 1;
		count = 62500;
	}
}
