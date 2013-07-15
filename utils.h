#ifndef __UTILS_H
#define __UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <avr/pgmspace.h>

/**__________________________________________________________
			General Bit manipulation Macros
**/
// Set Bit
#define sbi(Register, bit)    ((Register) |= (uint8_t)(1<<(bit)))
// Clear Bit
#define cbi(Register,bit)     ((Register) &=~(uint8_t)(1<<(bit)))
// Toggle Bit
#define tbi(Register, bit)    ((Register) ^= (uint8_t)(1<<(bit)))

// Bit is Set
#define bis(Register, Bit)    ( Register & (1<<Bit) )
// Bit is Clear
#define bic(Register, Bit)    (((Register & (1<<Bit))) ^ (1<<Bit))
 
/**__________________________________________________________
				Volatilize a variable
**/
#define vuint8(x)  (*(volatile uint8_t* )&(x))
#define vsint8(x)  (*(volatile int8_t*  )&(x))
#define vuint16(x) (*(volatile uint16_t*)&(x))
#define vsint16(x) (*(volatile int16_t* )&(x))
#define vuint32(x) (*(volatile uint32_t*)&(x))
#define vsint32(x) (*(volatile int32_t* )&(x))

/**__________________________________________________________
				Program space utilities
**/
// Like PSTR, but (D)ifferentiates pointer to .pgmspace from pointer to RAM by setting MSB of pointer
// This only works on AVR's with less than 0x8000 flash size
#define PSTRD(str)  ((char*)(0x8000+(int)(PSTR(str))))

/** __________________________________________________________
				Some useful functions
 **/


#ifdef __cplusplus
}
#endif

#endif


