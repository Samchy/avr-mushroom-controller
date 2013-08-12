#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "timer1.h"
#include "DHT22.h"
#include "lph7366.h"

#define DHT22_DEBUG	
#define DHT22_PIN_DEBUG

volatile uint16_t t1,t2,t3;

typedef enum {
		IDLE,
		START,			/* > 1ms */
		WAIT_ACK,		/* 20-40 us */
		ACK,			/* 80 us */
		WAIT_DATA_START,/* 80 us */
		WAIT_DATA, 		/* 50 us */
		DATA,			/* 26-28 us for "0" and 70 us for "1" */
		READY,			/* Data can be read by user */
} StateMachine_Type;

volatile StateMachine_Type State = IDLE;

volatile uint16_t prevICR;

DHT22_Info_Type DHT22_Info;

float DHT22_ReadTemperature()
{
	State = IDLE;
	return DHT22_Info.Temperature;
}

float DHT22_ReadHumidity()
{
	State = IDLE;
	return DHT22_Info.Humidity;
}

DHT22_STATE_Type DHT22_State(void)
{
	if( State == READY )
		return DHT22_READY;
	else if ( State == IDLE )
		return DHT22_IDLE;
	else 
		return DHT22_BUSY;
}

void DHT22_Read()
{	
	if( State != IDLE ); //return;

	State = START;

#ifdef DHT22_PIN_DEBUG
	cbi(PORTD,7);
	// clear the flag of ICP interrupt
	sbi(TIFR1, ICF1);
#endif
	// Configure the pin as GPIO out to send the Start signal
	sbi(DHT22_DIR, DHT22_PIN);
	cbi(DHT22_PORT, DHT22_PIN); // send start signal		

	// trigger compare interrupt in 1 ms
	OCR1B = clock() + 16000/*ms2tk(1.0)*/;
	sbi(TIMSK1, OCIE1B);
}

/**--------------------------------------------------------------------------------------------------
  Description	:  Timer1 COMPB vector - used for generating master signals in the DHT22 state machine
--------------------------------------------------------------------------------------------------**/
ISR (TIMER1_COMPB_vect)
{
#ifdef DHT22_PIN_DEBUG
	sbi(PORTD,7);cbi(PORTD,7);sbi(PORTD,7);cbi(PORTD,7);sbi(PORTD,7);
#endif
	if( State == START)
	{
		State = WAIT_ACK;
		//configure ICP pin as input with pullup
		cbi(DHT22_DIR, DHT22_PIN);
		sbi(DHT22_PORT, DHT22_PIN);
		// disable this interrupt
		cbi(TIMSK1, OCIE1B);
		// configure ICP interrupt
		sbi(TIMSK1, ICIE1);
		// select negative edge
		cbi(TCCR1B, ICES1); 
		// clear the flag of ICP interrupt
		sbi(TIFR1, ICF1);

		prevICR = clock();
	}
}

/**--------------------------------------------------------------------------------------------------
  Description	:  Timer1 CAPTURE vector - used to implement most of the DHT22 state machine
--------------------------------------------------------------------------------------------------**/
ISR(TIMER1_CAPT_vect)
{
	uint16_t currICR;
	uint16_t CAP;
	static uint64_t rxdata = 0;
	static uint8_t idx = 0;
	uint16_t temperature, humidity;
	uint8_t crc;
	int8_t sign;
#ifdef DHT22_DEBUG
	uint8_t errorstr[10];	
	uint8_t debug_t[15];
#endif

#ifdef DHT22_PIN_DEBUG
	tbi(PORTD,7);
#endif
	currICR = TCNT1;
	CAP = difftime_tk(currICR, prevICR) / 2;
	prevICR = currICR; 

	switch ( State ) {

		case WAIT_ACK:
			State = ACK;
			// select positive edge
			sbi(TCCR1B, ICES1); 
		break;	
				
		case ACK:	
			if( (CAP >= 80 - DHT22_TOLERANCE) && (CAP <= 80 + DHT22_TOLERANCE) )
			{
				State = WAIT_DATA_START;
				// select negative edge
				cbi(TCCR1B, ICES1); 
			}
			else
			{	
				State = IDLE;	
			}
		break;
					
		case WAIT_DATA_START:	
			if( (CAP >= 80 - DHT22_TOLERANCE) && (CAP <= 80 + DHT22_TOLERANCE) )
			{
				State = WAIT_DATA;
				// select positive edge
				sbi(TCCR1B, ICES1); 
			}
			else
			{
				State = IDLE;
			}
		break;
					
		case WAIT_DATA:	
			if( ( CAP >= 50 - DHT22_TOLERANCE) && ( CAP <= 50 + DHT22_TOLERANCE) )
			{
				State = DATA;
				// select negative edge
				cbi(TCCR1B, ICES1); 
			}
			else
			{	
				State = IDLE;	
			}
		break;
					
		case DATA:	
			// shift all bits to left to make space for the new one
			rxdata <<= 1;
			if( (CAP >= 27 - DHT22_TOLERANCE) && (CAP <= 27 + DHT22_TOLERANCE) )
			{
				State = WAIT_DATA;
				// select positive edge
				sbi(TCCR1B, ICES1); 
				idx++;
			}
			else if( (CAP >= 70 - DHT22_TOLERANCE) && (CAP <= 70 + DHT22_TOLERANCE) )
			{
				State = WAIT_DATA;
				// select positive edge
				sbi(TCCR1B, ICES1); 
				rxdata |= 1;
				idx++;
			}
			else
			{
				rxdata = 0;
				State = IDLE;
			}
			
			if( idx == 40 )
			{								
				crc = (uint8_t) rxdata;
				temperature = (uint16_t) (rxdata >> 8);
				humidity = (uint16_t) (rxdata >> 24);
				
				if( \
					( \
					(uint8_t) \
						( (uint8_t)(humidity) + (uint8_t)(humidity >> 8) \
						+ (uint8_t)(temperature) + (uint8_t)(temperature >> 8) \
					) \
					!= crc ) )
					{
						State = IDLE;
#ifdef DHT22_DEBUG
						dCursor(4,0);				
						dText("CRC ERROR");
#endif
					}
				
				else
				{
					if( temperature & 0x8000 )
						sign = -1;
					else
						sign = 1;
					temperature &= ~0x8000;
					DHT22_Info.Temperature = (float) temperature / 10 * sign;
					DHT22_Info.Humidity = (float) humidity / 10;
					
					State = READY;
					rxdata = 0;
					
					// Disable ICP interrupt
					cbi(TIMSK1, ICIE1);
					// clear the flag of ICP interrupt
					sbi(TIFR1, ICF1);
#ifdef DHT22_DEBUG
					dCursor(4,0);				
					dText("OK");
#endif
#ifdef DHT22_PIN_DEBUG
					tbi(PORTD,7);
#endif
				}
			}
		break;
	};
	
	// if an error occured, disable the interrupt
	if( State == IDLE )
	{
		idx = 0;
		rxdata = 0;

		// Disable ICP interrupt
		cbi(TIMSK1, ICIE1);
	}
}
