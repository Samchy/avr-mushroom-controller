/*_______________________________________________________________________

Controller:	ATmega 168P (Clock: 8 Mhz-internal)
Compiler:	AVR-GCC
Author:		Darius Berghe
Date:		July 2013
_________________________________________________________________________
*/
 
#ifndef F_CPU
	#define F_CPU 8000000UL
#endif

// #include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
// #include <avr/pgmspace.h>
#include "utils.h"
#include "timer1.h"
#include "rc5.h"
// #include "USART.h"
#include "cBuffer.h"
#include "lph7366.h"
#include "DHT22.h"


