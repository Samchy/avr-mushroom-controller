/*--------------------------------------------------------------------------------------------------
  Name         :  lph7366.h
  Description  :  Header file for LPH 7366 Nokia 84x48 graphic LCD driver.
  Author       :  26.03.2012 - Darius Berghe
--------------------------------------------------------------------------------------------------*/
#ifndef _LPH7366_H_
	#define _LPH7366_H_

#include "Main.h"
// #include <avr/pgmspace.h>

/*--------------------------------------------------------------------------------------------------
                                  Library submodules
--------------------------------------------------------------------------------------------------*/
#define LCD_with_TEXT
#define LCD_with_GRAPHICS

/*--------------------------------------------------------------------------------------------------
                           General purpose constants and operations
--------------------------------------------------------------------------------------------------*/
#define X_RES                  84
#define Y_RES                  48
 
// pin mapping
#define DC_PIN            	0x04  //  PD2
#define CE_PIN                 0x02  //  PD1
#define MOSI_PIN               0x08  //  PB3
#define RST_PIN                0x01  //  PD0
#define CLK_PIN                0x20  //  PB5

#define set_LCD_DC		(PORTD |= DC_PIN)
#define set_LCD_CE		(PORTD |= CE_PIN)
#define set_LCD_RST		(PORTD |= RST_PIN)
#define clr_LCD_DC		(PORTD &= ~DC_PIN)
#define clr_LCD_CE		(PORTD &= ~CE_PIN)
#define clr_LCD_RST		(PORTD &= ~RST_PIN)

#define LCD_CACHE_SIZE             ((X_RES * Y_RES) / 8)
#define LCD_LINES					( Y_RES / 8 )

/*--------------------------------------------------------------------------------------------------
                                       Type definitions
--------------------------------------------------------------------------------------------------*/
typedef enum {
	LCD_CMD , LCD_DATA
} LcdCmdData;

#ifdef LCD_with_GRAPHICS
typedef enum {
    PIXEL_OFF, PIXEL_ON, PIXEL_XOR 
} LcdPixelMode;
#endif

typedef struct  {
	 uint8_t  Cache [ LCD_CACHE_SIZE ];
	 int16_t  idx;
	 int16_t  lidx;
	 int16_t  hidx;
} LCD_t;

/*--------------------------------------------------------------------------------------------------
                                 Public function prototypes
                                 
						- Note	:	"d" prefix stands for "Display"
--------------------------------------------------------------------------------------------------*/
void initLCD     ( void );
void dContrast   ( uint8_t contrast );
void dClear      ( void );
void dRefresh    ( void );
int16_t dCachepos ( uint8_t x, uint8_t y );
uint8_t dPixelIsSet( uint8_t x, uint8_t y );

#ifdef LCD_with_TEXT
void dCursor     ( uint8_t line, uint8_t x );
void dChar  	 ( uint8_t ch );
void dText       ( uint8_t *dataPtr );
void dText_FF    ( const int8_t *dataPtr );
#define dText_P(conststr)  ( dText_FF(PSTR(conststr)) )
#endif

#ifdef LCD_with_GRAPHICS
void dPixel      ( uint8_t x, uint8_t y, LcdPixelMode mode );
void dLine       ( uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel_mode, uint8_t linewidth );
void dRectangle	 ( uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2 , uint8_t pixel_mode, uint8_t linewidth );
#endif

#endif  //  _LPH7366_H_
/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/
