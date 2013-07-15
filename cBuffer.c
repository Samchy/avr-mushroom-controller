#include "cBuffer.h"

/**--------------------------------------------------------------------------------------------------
  Name         :  cBufferInit
  Description  :  Initialize given buffer at given address with given size
  Argument(s)  :  Pointer to a cBufferType buffer, Pointer to the address this buffer is stored in memory
			   :  (user can allocate it in .data or .heap), Size of buffer
  Return value :  None.
--------------------------------------------------------------------------------------------------**/
void cBufferInit(cBufferType * buffer, int8_t * data, uint16_t size)
{
	buffer->ffilled = 0;
	buffer->fempty = 0;
	buffer->size = size;
	buffer->data = data;
}

/**--------------------------------------------------------------------------------------------------
  Name         :  cBufferIsFull
  Description  :  Function to check if a given cBuffer is full. 
  Argument(s)  :  Pointer to a cBufferType buffer
  Return value :  1 if FULL
--------------------------------------------------------------------------------------------------**/
uint8_t cBufferIsFull(cBufferType * buffer)
{
	return (buffer->fempty + 1) % buffer->size == buffer->ffilled;
}

/**--------------------------------------------------------------------------------------------------
  Name         :  cBufferIsEmpty.
  Description  :  Function to check if a given cBuffer is empty. 
  Argument(s)  :  Pointer to a cBufferType buffer.
  Return value :  1 if EMPTY.
--------------------------------------------------------------------------------------------------**/
uint8_t cBufferIsEmpty(cBufferType * buffer)
{ 
	return buffer->fempty == buffer->ffilled;
}

/**--------------------------------------------------------------------------------------------------
  Name         :  cBufferRead.
  Description  :  Function to get a char from front of given buffer.
  Argument(s)  :  Pointer to a cBufferType buffer.
  Return value :  character. (returns zero if buffer empty)
--------------------------------------------------------------------------------------------------**/
int8_t cBufferRead(cBufferType * buffer, int8_t * elem)
{
	if ( !cBufferIsEmpty(buffer) )
	{
		*elem = (int8_t) buffer->data[buffer->ffilled];
		buffer->ffilled = (buffer->ffilled + 1) % buffer->size;
		return 0; // success
	}
	else
		return -1; // buffer empty
}

/**--------------------------------------------------------------------------------------------------
  Name         :  cBufferReadAtIndex.
  Description  :  Function to get a char from given index of given buffer.
  Argument(s)  :  Pointer to a cBufferType buffer, Index.
  Return value :  character.
--------------------------------------------------------------------------------------------------**/
uint8_t cBufferReadAtIndex(cBufferType * buffer, uint16_t index)
{
	return buffer->data[ (buffer->ffilled + index) % (buffer->size) ];
}

/**--------------------------------------------------------------------------------------------------
  Name         :  cBufferWrite.
  Description  :  Function to add a char to the buffer.
			   :  \note Before calling this function, user is responsable for checking in a loop if provided buffer is not full.
			   :  \example 
				  while( cBufferFull(buffer) );
				  cBufferWrite(buffer,'c');
  Argument(s)  :  Pointer to a cBufferType buffer, character to add
  Return value :  0 - success; 1 - Buffer Full.
--------------------------------------------------------------------------------------------------**/
int8_t cBufferWrite(cBufferType * buffer, int8_t character)
{
	if ( !cBufferIsFull(buffer) )
	{
		buffer->data[buffer->fempty] = character;
		buffer->fempty = (buffer->fempty + 1) % buffer->size;
		return 0; // return success
	}
	else
	{
        return -1; // return -1 if Buffer Full
	}
}

/**--------------------------------------------------------------------------------------------------
  Name         :  cBufferFlush.
  Description  :  Function flush the buffer contents.
  Argument(s)  :  Pointer to a cBufferType buffer.
  Return value :  None.
--------------------------------------------------------------------------------------------------**/
void cBufferFlush(cBufferType * buffer)
{
    /// simulate an empty buffer
	buffer->ffilled = buffer->fempty;
}





