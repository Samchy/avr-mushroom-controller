#ifndef _RC5_H
#define _RC5_H

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "timer1.h"
#include "fifo.h"

// define the size of the circular buffer that will contain the received commands
#define RC5_CBUFFER_SIZE 5

#define UP 16
#define DOWN 17
#define LEFT 21
#define RIGHT 22
#define POWER 12
#define MENU 18
#define CHUP 32
#define CHDOWN 33
#define AV 56

/*** Macros to specify the sensor polarity. To be used as arguments for rc5init() ***/
#define RC5_NORMAL	0
#define RC5_INVERTED 	1

/*** RC5 protocol bits count ***/
#define RC5_TOTAL_BITS		14

/*** Mean values for short and long pulse durations in us(microseconds)  ***/
#define MEAN_SHORT_INTERVAL 	889
#define MEAN_LONG_INTERVAL		1778

/*** Tolerance from the mean value ***/
#define TOLERANCE	(MEAN_SHORT_INTERVAL / 4)

/*** Maximum and minimum values for short & long pulses  ***/
#define MIN_SHORT_INTERVAL		(MEAN_SHORT_INTERVAL - TOLERANCE)
#define MAX_SHORT_INTERVAL		(MEAN_SHORT_INTERVAL + TOLERANCE)
#define MIN_LONG_INTERVAL		(MEAN_LONG_INTERVAL - TOLERANCE)
#define MAX_LONG_INTERVAL		(MEAN_LONG_INTERVAL + TOLERANCE)

/***  MASK(BITS, SHIFT) returns a bitmask with a number of AMMOUNT bits set, shifted left by SHIFT positions. ***/
#define MASK(AMMOUNT, SHIFT) ( ((1<<AMMOUNT)-1) << SHIFT )

/*** Defining the bit regions within the 14-bit message to be received  ***/

#define S1_BIT_START		13
#define S1_BIT_COUNT		1
#define S1_BIT_VALUE		1

#define S2_BIT_START		12
#define S2_BIT_COUNT		1
#define S2_MASK MASK(S2_BIT_COUNT, S2_BIT_START)

#define TOGGLE_BIT_START	11
#define TOGGLE_BIT_COUNT	1
#define TOGGLE_MASK MASK(TOGGLE_BIT_COUNT, TOGGLE_BIT_START)

#define ADDRESS_BIT_START	5
#define ADDRESS_BIT_COUNT	5
#define ADDRESS_MASK MASK(ADDRESS_BIT_COUNT, ADDRESS_BIT_START)

#define COMMAND_BIT_START	0
#define COMMAND_BIT_COUNT	6
#define COMMAND_MASK MASK(COMMAND_BIT_COUNT, COMMAND_BIT_START)

/*** Used to shift S2 down to top of COMMAND (=6) ***/
#define XCOMMAND_SHIFT (S2_BIT_START - COMMAND_BIT_COUNT) 

typedef struct {
	uint8_t state	; // origin state (to be provided as argument to Transitions Table)
	uint16_t tcntval; // will hold the value of TCNT of the last ISR call
	uint8_t bits	; // number of bits currently occupied in data
} rc5context_t;

typedef struct {
	uint8_t senzorpolarity;
	volatile rc5context_t * context;
	uint16_t data;
} rc5_t;

typedef enum {
	start0,
	mid0,
	start1,
	mid1,
	error
} rc5state;

typedef enum {
	ShortPulse,
	LongPulse 
} rc5pulse;

// Type of the function which will contain the answer to the reveived message
// User is responsable to create it and provide a pointer to it as argument for rc5init() 
typedef void (*rc5FunctionToExecuteType)(uint16_t);

void rc5init (rc5FunctionToExecuteType, uint8_t);
void rc5on(void);
void rc5off(void);

void rc5exec (uint16_t data);
uint8_t rc5tog(uint16_t );
uint8_t rc5addr(uint16_t );
uint8_t rc5comm (uint16_t );
uint8_t rc5extcomm (uint16_t );

extern volatile rc5_t rc5;
extern volatile rc5context_t rc5context;
extern fifoType rc5buffer;

#ifdef __cplusplus
}
#endif

#endif
