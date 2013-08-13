#ifndef __USART_H
	#define __USART_H

#include <stdio.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include "fifo.h"

#define BAUD_RATE 38400UL
#define UBBRVAL (( F_CPU / BAUD_RATE / 16 ) - 1)
 
// Use a size of at least 3.
#define RXBUF_SIZE 10
#define TXBUF_SIZE 255

/** Initialisation routines for USART module **/
void initUART(void);

/** Function to send a char via USART **/
int16_t uartSendChar(int8_t, FILE *);

/** Function to send a string via USART **/
void uartSend(int8_t *);

/** Macro to send a string from flash via USART using uartSendFF() **/
#define uartSend_P(_str)	uartSendFF(PSTR(_str))
void uartSendFF(const int8_t * PROGMEM);

/** Function that will contain the user desired response.
	It is called at completion of a receival('\0' encountered)
**/
void uartResponse(fifoType *);

/** Receive a char routine **/
int16_t uartGet(FILE *);


#endif // header guard endif
