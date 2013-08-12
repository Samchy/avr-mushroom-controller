#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "timer1.h"
#include "DHT22.h"
#include "lph7366.h"

#define DHT22_DEBUG	
// #define DHT22_DEBUG_T
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

StateMachine_Type State = IDLE;

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

	// Configure the pin as GPIO out to send the Start signal
	sbi(DHT22_DIR, DHT22_PIN);
	cbi(DHT22_PORT, DHT22_PIN); // send start signal		
#ifdef DHT22_DEBUG_T
	t1 = clock();
#endif
	// trigger compare interrupt in 1 ms
	OCR1B = clock() + ms2tk(1);
	sbi(TIMSK1, OCIE1B);
}

/**--------------------------------------------------------------------------------------------------
  Description	:  Timer1 COMPB vector - used for generating master signals in the DHT22 state machine
--------------------------------------------------------------------------------------------------**/
ISR (TIMER1_COMPB_vect)
{
	uint8_t errorstr[10];
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

		prevICR = clock();
	}
}

/**--------------------------------------------------------------------------------------------------
  Description	:  Timer1 CAPTURE vector - used to implement most of the DHT22 state machine
--------------------------------------------------------------------------------------------------**/
ISR(TIMER1_CAPT_vect)
{
	uint16_t CAP, tcntval;
	float usCAP;
	static uint64_t rxdata = 0;
	static uint8_t idx = 0;
	uint16_t temperature, humidity;
	uint8_t crc;
	int8_t sign;
	uint8_t errorstr[10];	
	uint8_t debug_t[15];

	tcntval = TCNT1;
	CAP = difftime_tk(tcntval, prevICR); // time in ticks since last interrupt
	prevICR = tcntval; 
	usCAP = tk2us(CAP); // from now on we're working with us
/*
#ifdef DHT22_DEBUG

		
	dCursor(3,0); 
	sprintf(errorstr, "prev:%u", prevICR);
	dText(errorstr);
	dCursor(1,0); 
	sprintf(errorstr, "usCAP:%f", usCAP);
	dText(errorstr);
	dCursor(2,0);
	sprintf(errorstr, "S:%u", State);
	dText(errorstr);
	dCursor(4,0);
	sprintf(errorstr, "curr:%u", tcntval);
	dText(errorstr);
#endif
*/


	switch ( State ) {

		case WAIT_ACK:
			if( (usCAP >= 30 - DHT22_TOLERANCE) && (usCAP <= 30 + DHT22_TOLERANCE) )
			{ 
				State = ACK;
				// select positive edge
				sbi(TCCR1B, ICES1); 
			}
			else
				State = IDLE;	
		break;	
				
		case ACK:	
			if( (usCAP >= 80 - DHT22_TOLERANCE) && (usCAP <= 80 + DHT22_TOLERANCE) )
			{
				State = WAIT_DATA_START;
				// select negative edge
				cbi(TCCR1B, ICES1); 
			}
			else
				State = IDLE;
		break;
					
		case WAIT_DATA_START:	
			if( (usCAP >= 80 - DHT22_TOLERANCE) && (usCAP <= 80 + DHT22_TOLERANCE) )
			{
				State = WAIT_DATA;
				// select positive edge
				sbi(TCCR1B, ICES1); 
			}
			else
				State = IDLE;
		break;
					
		case WAIT_DATA:	
			if( ( usCAP >= 50 - DHT22_TOLERANCE) && ( usCAP <= 50 + DHT22_TOLERANCE) )
			{
				State = DATA;
				// select negative edge
				cbi(TCCR1B, ICES1); 
			}
			else
				State = IDLE;
		break;
					
		case DATA:	
			// shift all bits to left to make space for the new one
			rxdata <<= 1;
			if( (usCAP >= 27 - DHT22_TOLERANCE) && (usCAP <= 27 + DHT22_TOLERANCE) )
			{
				State = WAIT_DATA;
				// select positive edge
				sbi(TCCR1B, ICES1); 
				idx++;
			}
			else if( (usCAP >= 70 - DHT22_TOLERANCE) && (usCAP <= 70 + DHT22_TOLERANCE) )
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
#ifdef DHT22_DEBUG
				dCursor(5,0);				
				dText("MSG RECV");
#endif
				
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
						dCursor(5,0);				
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
#ifdef DHT22_DEBUG
				dCursor(5,0);				
				dText("OK");
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

#ifdef DHT22_DEBUG
		dCursor(5,0);	
		dText("ERROR");
		dRefresh();
#endif

		// Disable ICP interrupt
		cbi(TIMSK1, ICIE1);
	}
}
