#include "rc5.h"

/*** Private functions ***/
void rc5softinit(uint8_t senzorpolarity);

/*** Global Variables ***/
fifoType rc5buffer;
int8_t rc5cBuffer[RC5_CBUFFER_SIZE] ;

volatile rc5_t rc5;
volatile rc5context_t rc5context;
volatile rc5FunctionToExecuteType rc5FunctionToExecute;

/** 
   The following matrix should be used like Transition[ PulseType ][ OriginState ]
   It will return the new state. 
EX:If OriginState == start0 AND measured pulse is long, accessing the matrix like that
   is actually Transision[1][0], which returns the error state. etc. 
**/
volatile const uint8_t Transision[][5] __attribute__ ((progmem)) = {
	{ // short pulse
		mid0, 
		start0, 
		mid1, 
		start1, 
		error,
	},
	{ // long pulse
		error, 
		mid1, 
		error, 
		mid0,
		error
	}
};

/**-------------------------------------------------------------------------------------------------
  Description         :  Function to hold the appropriate answer to a received message
-------------------------------------------------------------------------------------------------**/
void rc5exec(uint16_t data)
{
 	uint16_t command = rc5comm(data);
	rc5FunctionToExecute(data);
}

/**-------------------------------------------------------------------------------------------------
  Description         :  RC5 initialization function.
-------------------------------------------------------------------------------------------------**/
void rc5init (rc5FunctionToExecuteType function, uint8_t senzorpolarity)
{
	fifoInit(&rc5buffer, rc5cBuffer, RC5_CBUFFER_SIZE);
		
	cbi(DDRD,3); // PD3(INT1) 
  	sbi(PORTD,3); // pullup
	rc5FunctionToExecute = function;	
	rc5.senzorpolarity = senzorpolarity;
	rc5.context = (volatile rc5context_t *) &rc5context;
	
	rc5softinit(senzorpolarity);
	rc5on();
}

/**-------------------------------------------------------------------------------------------------
  Description        :  A lightweight (RE)initialization routine called when an error state occurs
						or when a message was successfully received.
-------------------------------------------------------------------------------------------------**/
void rc5softinit(uint8_t senzorpolarity)
{
	if(senzorpolarity == RC5_NORMAL)
	{
		EICRA |= (1<<ISC10) | (1<<ISC11); // Interrupt on Rising Edge since the Start Bit of RC-5 Protocol is a logic 0 to 1 transition
	}
	else if (senzorpolarity == RC5_INVERTED)
	{
		EICRA &= ~(1<<ISC10);
		EICRA |= (1<<ISC11); // Interrupt on Falling Edge (senzor inverts the signal: a pulse corresponds to a low level output)
	}
	
	rc5.context->bits = 0;
	rc5.data = 0;
	
	// this is application specific. read the comment in ISR for the first call
	// TIMSK3 |= (1<<OCIE3A); // en TIMER3_COMPA interrupt
}

/**-------------------------------------------------------------------------------------------------
  Description         :  Turns ON the interrupt that decodes the message
-------------------------------------------------------------------------------------------------**/
void rc5on(void)
{
	EIMSK |= (1<<INT1); // Enable interrupt
}

/**-------------------------------------------------------------------------------------------------
  Description         :  Turns OFF the interrupt that decodes the message
-------------------------------------------------------------------------------------------------**/
void rc5off(void)
{
	EIMSK &= ~(1<<INT1); // Disable interrupt
}

/**-------------------------------------------------------------------------------------------------
  Description         :  Returns the Rc5 protocol toggle bit if given the full message
-------------------------------------------------------------------------------------------------**/
uint8_t rc5tog(uint16_t message)
{
  return (uint8_t)((message & TOGGLE_MASK) >> TOGGLE_BIT_START);
}

/**-------------------------------------------------------------------------------------------------
  Description         :  Returns the RC5 protocol address field if given the full message
-------------------------------------------------------------------------------------------------**/
uint8_t rc5addr(uint16_t message)
{
  return (uint8_t)((message & ADDRESS_MASK) >> ADDRESS_BIT_START);
}

/**-------------------------------------------------------------------------------------------------
  Description         :  Returns the RC5 protocol command field if given the full message
-------------------------------------------------------------------------------------------------**/
uint8_t rc5comm (uint16_t message)
{
  return (uint8_t)((message & COMMAND_MASK) >> COMMAND_BIT_START);
}

/**-------------------------------------------------------------------------------------------------
  Description         :  Returns the RC5 protocol extended command field if given the full message
-------------------------------------------------------------------------------------------------**/
uint8_t rc5extcomm (uint16_t message)
{
  return (uint8_t)(rc5comm(message) | ((message & S2_MASK) >> XCOMMAND_SHIFT));
}

#include "lph7366.h"
/**-------------------------------------------------------------------------------------------------
  Description         :  INT1 interrupt - used to measure time intervals, insert them into the state machine
						 and build up the decoded message if no error occurs. 
-------------------------------------------------------------------------------------------------**/
ISR(INT1_vect)
{
	volatile uint16_t tcntval = clock();
	volatile uint16_t signalwidth = tcntval - rc5.context->tcntval;
	volatile uint8_t state;

	/** First ISR call in a message stream **/
	if( rc5.context->bits == 0 ) 
	{
		state = mid1;
	}
	
	/** The rest of ISR calls **/	
	else 
	{
		/** Short Pulse **/
		if  ( ( signalwidth >= us2tk(MIN_SHORT_INTERVAL)) && ( signalwidth <= us2tk(MAX_SHORT_INTERVAL)) ) 
			state = pgm_read_byte( & Transision[ ShortPulse ][ rc5.context->state ]);
		
		/** Long Pulse **/
		else if ( (signalwidth >= us2tk(MIN_LONG_INTERVAL)) && ( signalwidth <= us2tk(MAX_LONG_INTERVAL)) )
			state = pgm_read_byte( & Transision[ LongPulse ][ rc5.context->state ]);
		
		/** Signal period measured not within limits <=> error **/
		else
			state = error;
	}
	
	/** Add the received bit if no error detected **/
	if ( state == mid0 || state == mid1 )
	{
		rc5.data <<= 1;
		if ( state == mid1 )
			rc5.data |= 1;	
		rc5.context->bits++;
	}
	
	/** Message successfully received **/
	if ( rc5.context->bits == 14 && state != error )
	{
 		rc5exec(rc5.data);
		rc5softinit(rc5.senzorpolarity);
		return;
	}
	
	if( state != error )
	{
		// prepare next ISR
		rc5.context->state = state;
		rc5.context->tcntval = tcntval;
		tbi(EICRA,ISC10); // change interrupt polarity
	}

	/** If error detected, reset everything **/
	if (state == error)
	{
		// write an error code - for debugging purposes
		fifoWrite(&rc5buffer, 104);	
		// enable the OC1A compare interrupt. when it triggers, it means the current message stream has ended an RC5 state
		// machine can be safely reset. otherwise, the remaining bits would mess it and you dont want that.
		OCR1A = clock() + ms2tk( 1 + (RC5_TOTAL_BITS - rc5.context->bits) * MEAN_LONG_INTERVAL / 1000 ); 
		TIMSK1 |= (1<<OCIE1A); // enable interrupt
	}
}

/**--------------------------------------------------------------------------------------------------
  Description	:  Timer1 COMPA vector - used to detect when the IR bit stream ended
--------------------------------------------------------------------------------------------------**/
ISR (TIMER1_COMPA_vect)
{
	// reset the IR state machine
	rc5.data = 0x0000;
 	rc5softinit(rc5.senzorpolarity);
	TIMSK1 &= ~(1<<OCIE1A); 
}
