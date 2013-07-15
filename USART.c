
#include "USART.h"

#if defined(__AVR_ATmega48__) || defined(__AVR_ATmega88__) ||\
    defined(__AVR_ATmega168__)
/* Convert ATmega8/16 names to ATmegaX8 names */
#define USART_RXC_vect USART_RX_vect
#define UDR     UDR0
#define UCSRA   UCSR0A
#define UCSRB   UCSR0B
#define FE      FE0
#define TXEN    TXEN0
#define RXEN    RXEN0
#define RXCIE   RXCIE0
#define UDRE    UDRIE0
#define U2X     U2X0
#define UBRRL   UBRR0L
#define UBRRH 	UBRR0H
#define RXC	  	RXC0
#define UCSRC 	UCSR0C
#define UCSZ0 	UCSZ00
#define UCSZ1	UCSZ01
#define UMSEL0 	UMSEL00
#define UMSEL1  UMSEL01
#endif

/** __________________________________________________________________
		Initialisation of USART module buffers as circular buffers
 **/
 cBufferType rxbuffer ;
 cBufferType txbuffer ;
  
 cBufferType * rxbuf ;
 cBufferType * txbuf ;

 /** Receive and Transmit actual buffer arrays **/
 int8_t USART_rxBuffer[RXBUF_SIZE] ;
 int8_t USART_txBuffer[TXBUF_SIZE] ;
 
/**-------------------------------------------------------------------------------------------------
  Name         :  initUSART
  Description  :  initialises the USART  
  Argument(s)  :  None.
  Return value :  None.
-------------------------------------------------------------------------------------------------**/
void initUSART()
{
	UCSRB |= (1<<RXEN) | (1<<TXEN) ; // enable reception & transmission circuitry
	
	UBRRH = (uint8_t)(UBBRVAL>>8); // Write the value of Prescaler into UBBR Registers
	UBRRL = (uint8_t)(UBBRVAL   );
	
#if defined(__AVR_ATmega48__) || defined(__AVR_ATmega88__) ||\
    defined(__AVR_ATmega168__)
	UCSRC &= ~ (3<<UMSEL0); // asynchronous mode
#elif defined(__AVR_ATmega8__) || defined(__AVR_ATmega16__) ||\
    defined(__AVR_ATmega32__)
	UCSRC |= (1<<URSEL) | (3<<UCSZ0); // 1 stop bit	and 	8-bit data frame, no parity
	UCSRC &= ~ (1<<UMSEL); // asynchronous mode
#endif

	UCSRB |= (1<<RXCIE); // Enable RXC Interrupt

	rxbuf = &rxbuffer;
	txbuf = &txbuffer;
	
	//Alternatvely: uint8_t * USART_rxBuffer = (uint8_t *) malloc(RXBUF_SIZE * sizeof(uint8_t));
	//			    uint8_t * USART_txBuffer = (uint8_t *) malloc(TXBUF_SIZE * sizeof(uint8_t)); 
	
	cBufferInit( rxbuf, USART_txBuffer, RXBUF_SIZE);
	cBufferInit( txbuf, USART_txBuffer, TXBUF_SIZE);
}

/**-------------------------------------------------------------------------------------------------
  Name         :  usartSendChar
  Description  :  Loads character to buffer
  Argument(s)  :  character to send, stream pointer
  Return value :  0 for success
-------------------------------------------------------------------------------------------------**/
int16_t usartSendChar(int8_t c, FILE * stream)
{
	if (c == '\n')
		usartSendChar('\r', stream); // manually insert CR before LF 
	if ( BitIsSet(UCSRB,UDRE) && BitIsSet(SREG,7))	
		while( cBufferIsFull( txbuf ) );
	cBufferWrite(txbuf, c);
	UCSRB |= (1<<UDRE); // enable UDRE interrupt
	return 0;
}

/**-------------------------------------------------------------------------------------------------
  Name         :  usartSend
  Description  :  Sends a string from RAM.  
  Argument(s)  :  pointer to string in RAM.
  Return value :  None.
-------------------------------------------------------------------------------------------------**/
void usartSend( int8_t * string )
{
	int8_t * ptr;
	for( ptr = string; *ptr != '\0'; ptr++ )
		{
			// If interrupt is not active (and TXBUF_SIZE is smaller than desired string to send),
			// the parent 'for' loop will fill the buffer thus leading to an infinite while loop
			// (UDRE ISR cannot make space in buffer). So we ask if Interrupt is active first.
			if (BitIsSet(UCSRB,UDRE) && BitIsSet(SREG,7))	
				while( cBufferIsFull( txbuf ) );
			else
				UCSRB |= (1<<UDRE); // enable UDRE interrupt
			cBufferWrite(txbuf, *ptr);
		}
}

/**-------------------------------------------------------------------------------------------------
  Name         :  usartSendFF (FF - From Flash)
  Description  :  Sends a string from .pgmspace  
  Argument(s)  :  pointer to string in .pgmspace
  Return value :  None.
  \note		   :  Use usartSend_P macro from header file instead (or simply printf_P)
-------------------------------------------------------------------------------------------------**/
void usartSendFF(const int8_t * stringFF)
{
	const int8_t * ptr = stringFF;
	for( ; pgm_read_byte(ptr) != '\0'; ptr++ )
		{
			if (BitIsSet(UCSRB,UDRE) && BitIsSet(SREG,7))	
				while( cBufferIsFull( txbuf ) );
			else
				UCSRB |= (1<<UDRE); // enable UDRE interrupt
			cBufferWrite(txbuf, pgm_read_byte(ptr) );
		}
}

/**-------------------------------------------------------------------------------------------------
  Name         :  usartGet
  Description  :  waits for an RXC interrupt and returns received value
  Argument(s)  :  pointer to a stream
  Return value :  received character
-------------------------------------------------------------------------------------------------**/
int16_t usartGet(FILE * stream) 
{
	int8_t character;
	if( BitIsSet(UCSRB,RXCIE) && BitIsSet(SREG,7));
		while ( cBufferIsEmpty(rxbuf) )
	// RXC Interrupt breaks this loop
		;
	cBufferRead(rxbuf,&character);
	return character;
}

/**-------------------------------------------------------------------------------------------------
  Description         :  UDR Empty Interrupt
-------------------------------------------------------------------------------------------------**/
ISR( USART_UDRE_vect )
{
	cBufferRead( txbuf, &UDR );
	if ( cBufferIsEmpty(txbuf) )
		UCSRB &= ~( 1<<UDRE ); // disable UDRE interrupt
	// Yea. Simple as that.
}

/**-------------------------------------------------------------------------------------------------
  Description         :  Rx Complete Interrupt
-------------------------------------------------------------------------------------------------**/
ISR(USART_RXC_vect)
{
	int8_t c = UDR; // read the value of the 8-bit UDR buffer
	while ( cBufferIsFull(rxbuf) )
	 // if buffer is full, you'll lose all incoming characters. 
	// Normally, if you just use scanf, getchar etc. they'll read the buffer for you so this is not a problem.
		;
	cBufferWrite(rxbuf,c); // write it to our RAM buffer
}