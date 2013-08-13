#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "config.h"
#include "utils.h"

#if defined(__AVR_ATmega48__) || defined(__AVR_ATmega88__) ||\
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__)
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
		Initialisation of UART module buffers as circular buffers
 **/
 fifoType rxbuffer ;
 fifoType txbuffer ;
  
 fifoType * rxbuf ;
 fifoType * txbuf ;

 /** Receive and Transmit actual buffer arrays **/
 int8_t UART_rxBuffer[RXBUF_SIZE] ;
 int8_t UART_txBuffer[TXBUF_SIZE] ;
 
/**-------------------------------------------------------------------------------------------------
  Name         :  initUART
  Description  :  initialises the UART  
  Argument(s)  :  None.
  Return value :  None.
-------------------------------------------------------------------------------------------------**/
void initUART()
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
	
	//Alternatvely: uint8_t * UART_rxBuffer = (uint8_t *) malloc(RXBUF_SIZE * sizeof(uint8_t));
	//			    uint8_t * UART_txBuffer = (uint8_t *) malloc(TXBUF_SIZE * sizeof(uint8_t)); 
	
	fifoInit( rxbuf, UART_txBuffer, RXBUF_SIZE);
	fifoInit( txbuf, UART_txBuffer, TXBUF_SIZE);
}

/**-------------------------------------------------------------------------------------------------
  Name         :  uartSendChar
  Description  :  Loads character to buffer
  Argument(s)  :  character to send, stream pointer
  Return value :  0 for success
-------------------------------------------------------------------------------------------------**/
int16_t uartSendChar(int8_t c, FILE * stream)
{
	if (c == '\n')
		uartSendChar('\r', stream); // manually insert CR before LF 
	if ( bis(UCSRB,UDRE) && bis(SREG,7))	
		while( fifoIsFull( txbuf ) );
	fifoWrite(txbuf, c);
	UCSRB |= (1<<UDRE); // enable UDRE interrupt
	return 0;
}

/**-------------------------------------------------------------------------------------------------
  Name         :  uartSend
  Description  :  Sends a string from RAM.  
  Argument(s)  :  pointer to string in RAM.
  Return value :  None.
-------------------------------------------------------------------------------------------------**/
void uartSend( int8_t * string )
{
	int8_t * ptr;
	for( ptr = string; *ptr != '\0'; ptr++ )
		{
			// If interrupt is not active (and TXBUF_SIZE is smaller than desired string to send),
			// the parent 'for' loop will fill the buffer thus leading to an infinite while loop
			// (UDRE ISR cannot make space in buffer). So we ask if Interrupt is active first.
			if (bis(UCSRB,UDRE) && bis(SREG,7))	
				while( fifoIsFull( txbuf ) );
			else
				UCSRB |= (1<<UDRE); // enable UDRE interrupt
			fifoWrite(txbuf, *ptr);
		}
}

/**-------------------------------------------------------------------------------------------------
  Name         :  uartSendFF (FF - From Flash)
  Description  :  Sends a string from .pgmspace  
  Argument(s)  :  pointer to string in .pgmspace
  Return value :  None.
  \note		   :  Use uartSend_P macro from header file instead (or simply printf_P)
-------------------------------------------------------------------------------------------------**/
void uartSendFF(const int8_t * stringFF)
{
	const int8_t * ptr = stringFF;
	for( ; pgm_read_byte(ptr) != '\0'; ptr++ )
		{
			if (bis(UCSRB,UDRE) && bis(SREG,7))	
				while( fifoIsFull( txbuf ) );
			else
				UCSRB |= (1<<UDRE); // enable UDRE interrupt
			fifoWrite(txbuf, pgm_read_byte(ptr) );
		}
}

/**-------------------------------------------------------------------------------------------------
  Name         :  uartGet
  Description  :  waits for an RXC interrupt and returns received value
  Argument(s)  :  pointer to a stream
  Return value :  received character
-------------------------------------------------------------------------------------------------**/
int16_t uartGet(FILE * stream) 
{
	int8_t character;
	if( bis(UCSRB,RXCIE) && bis(SREG,7));
		while ( fifoIsEmpty(rxbuf) )
	// RXC Interrupt breaks this loop
		;
	fifoRead(rxbuf,&character);
	return character;
}

/**-------------------------------------------------------------------------------------------------
  Description         :  UDR Empty Interrupt
-------------------------------------------------------------------------------------------------**/
ISR( USART_UDRE_vect )
{
	fifoRead( txbuf, &UDR );
	if ( fifoIsEmpty(txbuf) )
		UCSRB &= ~( 1<<UDRE ); // disable UDRE interrupt
	// Yea. Simple as that.
}

/**-------------------------------------------------------------------------------------------------
  Description         :  Rx Complete Interrupt
-------------------------------------------------------------------------------------------------**/
ISR(USART_RXC_vect)
{
	int8_t c = UDR; // read the value of the 8-bit UDR buffer
	while ( fifoIsFull(rxbuf) )
	 // if buffer is full, you'll lose all incoming characters. 
	// Normally, if you just use scanf, getchar etc. they'll read the buffer for you so this is not a problem.
		;
	fifoWrite(rxbuf,c); // write it to our RAM buffer
}
