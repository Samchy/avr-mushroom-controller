#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "backlight.h"

uint8_t gBacklight;

void initBacklight(void)
{
	sbi(DDRB,1);
	sbi(PORTB,1);
	TCCR2A = 0;
	TCCR2B = 1;
	TIMSK2 = 3;
	setBacklight(7);
}

void setBacklight(uint8_t val)
{
	if(val > 15) return;
	if(val > 0) 
	{
		if(gBacklight == 0)
			TIMSK2 = 3;
		OCR2A = val * 16 - 1;
	}
	else
	{
		TIMSK2 = 0;
		cbi(PORTB,1);
	}

	gBacklight = val;
}

uint8_t getBacklight(void)
{
	return gBacklight;
}

void incrBacklight(void)
{
	if( gBacklight < 15 )
		setBacklight(gBacklight+1);
}

void decrBacklight(void)
{
	if( gBacklight > 0 )
		setBacklight(gBacklight-1);
}

ISR(TIMER2_COMPA_vect)
{
	cbi(PORTB,1);
}

ISR(TIMER2_OVF_vect)
{
	sbi(PORTB,1);
}
