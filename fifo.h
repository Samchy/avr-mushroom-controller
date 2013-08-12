/**  
____________________________________________________________________________

File Name	: 'cbuffers.h'
Title		: Multipurpose (byte) circular buffer structure and routines.
Author		: Darius Berghe
Created		: 07/03/2012
Revised		: 17/03/2012
Version		: 1.01
Editor Tabs	: 4
_____________________________________________________________________________
**/
#ifndef __CBUFFERS_H
	#define __CBUFFERS_H
	
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    int8_t * data;			///< pointer to memory area where the buffer will be stored
	int16_t ffilled;			///< the index where the data starts (\note Read it as 'first-filled-slot')
	int16_t fempty;			///< the index where empty/non-used area starts (\note Read it as 'first-empty-slot')
	uint16_t size;			///< the size of buffer
} fifoType;

/**	Initialize given buffer at given address with given size **/
void fifoInit(fifoType *, int8_t *, uint16_t);

/** Function to check if a given fifo is full. It returns 1 if it's full. **/
uint8_t fifoIsFull(fifoType *);

/** Function to check if a given fifo is empty. It returns 1 if it's empty. **/
uint8_t fifoIsEmpty(fifoType *);

/** Function to get a char from front of given buffer. **/
int8_t fifoRead(fifoType *, int8_t *);

/** Function to get a char from given index of given buffer. **/
uint8_t fifoReadAtIndex(fifoType *, uint16_t);

/** Function to add a char at end of given buffer **/
int8_t fifoWrite(fifoType *, int8_t);

/** Flush the contents of a given buffer **/
void fifoFlush(fifoType * );

#ifdef __cplusplus
}
#endif

#endif


