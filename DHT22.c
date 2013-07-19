#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "timer1.h"
#include "DHT22.h"

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

void DHT22_Read()
{	
	// Configure the pin as GPIO to send the Start signal
	sbi(DHT22_DIR, DHT22_PIN);
	cbi(DHT22_PORT, DHT22_PIN); // send start signal	

	State = START;	

	// interrupt at 1 ms
	OCR1B = clock() + ms2tk(1);
	sbi(TIMSK1, OCIE1B);
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

/**--------------------------------------------------------------------------------------------------
  Description	:  Timer1 COMPB vector - used to generate a 1ms pause in the DHT22 state machine
--------------------------------------------------------------------------------------------------**/
ISR (TIMER1_COMPB_vect)
{
	// disable this interrupt
	TIMSK1 &= ~(1<<OCIE1B);

	if( State == START )
	{
		State = WAIT_ACK;
		prevICR = clock();
		// configure ICP interrupt
		sbi(TIMSK1, ICIE1);
		// select negative edge
		cbi(TCCR1B, ICES1); 
	}
}

/**--------------------------------------------------------------------------------------------------
  Description	:  Timer1 CAPTURE vector - used to implement most of the DHT22 state machine
--------------------------------------------------------------------------------------------------**/
ISR(TIMER1_CAPT_vect)
{
	uint16_t CAP;
	static uint64_t rxdata = 0;
	static uint8_t idx = 0;
	uint16_t temperature, humidity;
	uint8_t crc;
	int8_t sign;
	
	cbi(TIFR1, ICF1); // clear flag for this interrupt
	CAP = tk2ms(ICR1 - prevICR);
	prevICR = CAP;
	
	switch ( State ) {
		case WAIT_ACK:
					if( CAP <= 40 )
					{
						State = ACK;
						// select positive edge
						sbi(TCCR1B, ICES1); 
					}
					else
						State = IDLE;
		break;
					
		case ACK:	
					if( (CAP >= 80 - TOLERANCE) && (CAP <= 80 + TOLERANCE) )
					{
						State = WAIT_DATA_START;
						// select negative edge
						cbi(TCCR1B, ICES1); 
					}
					else
							State = IDLE;
		break;
					
		case WAIT_DATA_START:	
					if( (CAP >= 80 - TOLERANCE) && (CAP <= 80 + TOLERANCE) )
					{
						State = WAIT_DATA;
						// select positive edge
						sbi(TCCR1B, ICES1); 
					}
					else
							State = IDLE;
		break;
					
		case WAIT_DATA:	
					if( ( CAP >= 50 - TOLERANCE) && ( CAP <= 50 + TOLERANCE) )
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
					if( (CAP >= 27 - TOLERANCE) && (CAP <= 27 + TOLERANCE) )
					{
						State = WAIT_DATA;
						// select positive edge
						sbi(TCCR1B, ICES1); 
						idx++;
					}
					else if( (CAP >= 70 - TOLERANCE) && (CAP <= 70 + TOLERANCE) )
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
							
							State = IDLE;
						
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
