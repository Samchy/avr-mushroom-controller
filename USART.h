#ifndef __USART_H
	#define __USART_H

#include <stdio.h>
#include <stdint.h>
#include <avr/pgmspace.h>


#include "Main.h"
#include "cBuffer.h"

#define BAUD_RATE 38400UL
#define UBBRVAL (( F_CPU / BAUD_RATE / 16 ) - 1)
 
// Use a size of at least 3.
#define RXBUF_SIZE 10
#define TXBUF_SIZE 255

/** Initialisation routines for USART module **/
void initUSART(void);

/** Function to send a char via USART **/
int16_t usartSendChar(int8_t, FILE *);

/** Function to send a string via USART **/
void usartSend(int8_t *);

/** Macro to send a string from flash via USART using usartSendFF() **/
#define usartSend_P(_str)	usartSendFF(PSTR(_str))
void usartSendFF(const int8_t * PROGMEM);

/** Function that will contain the user desired response.
	It is called at completion of a receival('\0' encountered)
**/
void usartResponse(cBufferType *);

/** Receive a char routine **/
int16_t usartGet(FILE *);


#endif // header guard endif
