/*--------------------------------------------------------------------------------------------------
  Name         :  lph7366.c
  Description  :  This is a driver for the LPH - 7366 Nokia 84x48 graphic LCD.
  Author       :  2012-03-26 - Darius Berghe
--------------------------------------------------------------------------------------------------*/
#include <stdint.h>
#include "lph7366.h"


/*--------------------------------------------------------------------------------------------------
                                Private function prototypes
--------------------------------------------------------------------------------------------------*/
static void dSend ( uint8_t data, LcdCmdData cd );

#ifdef LCD_with_GRAPHICS
static void RectangleCoordAdjust( uint8_t * x1, uint8_t * y1, uint8_t * x2, uint8_t * y2, uint8_t * ulc, uint8_t * lrc);
static int8_t min(int8_t el1, int8_t el2);
static int8_t max(int8_t el1, int8_t el2);
#endif

#ifdef LCD_with_TEXT
/*--------------------------------------------------------------------------------------------------
  This table defines the standard ASCII characters in a 5x7 dot format. It is stored in .progmem.
--------------------------------------------------------------------------------------------------*/
// char FontTable[1][1];

static const uint8_t FontTable [][5] PROGMEM =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },  // space
    { 0x00, 0x00, 0x2f, 0x00, 0x00 },   // !
    { 0x00, 0x07, 0x00, 0x07, 0x00 },   // "
    { 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // #
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $
    { 0xc4, 0xc8, 0x10, 0x26, 0x46 },   // %
    { 0x36, 0x49, 0x55, 0x22, 0x50 },   // &
    { 0x00, 0x05, 0x03, 0x00, 0x00 },   // '
    { 0x00, 0x1c, 0x22, 0x41, 0x00 },   // (
    { 0x00, 0x41, 0x22, 0x1c, 0x00 },   // )
    { 0x14, 0x08, 0x3E, 0x08, 0x14 },   // *
    { 0x08, 0x08, 0x3E, 0x08, 0x08 },   // +
    { 0x00, 0x00, 0x50, 0x30, 0x00 },   // ,
    { 0x10, 0x10, 0x10, 0x10, 0x10 },   // -
    { 0x00, 0x60, 0x60, 0x00, 0x00 },   // .
    { 0x20, 0x10, 0x08, 0x04, 0x02 },   // /
    { 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0
    { 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1
    { 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2
    { 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3
    { 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4
    { 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6
    { 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7
    { 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8
    { 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9
    { 0x00, 0x36, 0x36, 0x00, 0x00 },   // :
    { 0x00, 0x56, 0x36, 0x00, 0x00 },   // ;
    { 0x08, 0x14, 0x22, 0x41, 0x00 },   // <
    { 0x14, 0x14, 0x14, 0x14, 0x14 },   // =
    { 0x00, 0x41, 0x22, 0x14, 0x08 },   // >
    { 0x02, 0x01, 0x51, 0x09, 0x06 },   // ?
    { 0x32, 0x49, 0x59, 0x51, 0x3E },   // @
    { 0x7E, 0x11, 0x11, 0x11, 0x7E },   // A
    { 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B
    { 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C
    { 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D
    { 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E
    { 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F
    { 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G
    { 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H
    { 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I
    { 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J
    { 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K
    { 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M
    { 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N
    { 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O
    { 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P
    { 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q
    { 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R
    { 0x46, 0x49, 0x49, 0x49, 0x31 },   // S
    { 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T
    { 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U
    { 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V
    { 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W
    { 0x63, 0x14, 0x08, 0x14, 0x63 },   // X
    { 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y
    { 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z
    { 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [
    { 0x02, 0x04, 0x08, 0x10, 0x20 },   // backslash
    { 0x00, 0x41, 0x41, 0x7f, 0x00 },   // ]
    { 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^
    { 0x40, 0x40, 0x40, 0x40, 0x40 },   // _
    { 0x00, 0x01, 0x02, 0x04, 0x00 },   // '
    { 0x20, 0x54, 0x54, 0x54, 0x78 },   // a
    { 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b
    { 0x38, 0x44, 0x44, 0x44, 0x20 },   // c
    { 0x38, 0x44, 0x44, 0x48, 0x7F },   // d
    { 0x38, 0x54, 0x54, 0x54, 0x18 },   // e
    { 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f
    { 0x0C, 0x52, 0x52, 0x52, 0x3E },   // g
    { 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h
    { 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i
    { 0x20, 0x40, 0x44, 0x3D, 0x00 },   // j
    { 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k
    { 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l
    { 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m
    { 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n
    { 0x38, 0x44, 0x44, 0x44, 0x38 },   // o
    { 0x7C, 0x14, 0x14, 0x14, 0x08 },   // p
    { 0x08, 0x14, 0x14, 0x18, 0x7C },   // q
    { 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r
    { 0x48, 0x54, 0x54, 0x54, 0x20 },   // s
    { 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t
    { 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u
    { 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v
    { 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w
    { 0x44, 0x28, 0x10, 0x28, 0x44 },   // x
    { 0x0C, 0x50, 0x50, 0x50, 0x3C },   // y
    { 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z
    { 0x00, 0x08, 0x36, 0x41, 0x00 },   // {
    { 0x00, 0x00, 0x7f, 0x00, 0x00 },   // |
    { 0x00, 0x41, 0x36, 0x08, 0x00 },   // }
    { 0x04, 0x02, 0x04, 0x08, 0x04 }    // ~
};

#endif
/*--------------------------------------------------------------------------------------------------
                                      Global Variables
--------------------------------------------------------------------------------------------------*/
LCD_t LCD;

/*--------------------------------------------------------------------------------------------------
  Name         :  LcdInit
  Description  :  Performs MCU SPI & LCD controller initialization.
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void initLCD ( void )
{

    //  Set output bits on port B.
    DDRB |= RST_PIN | DC_PIN | CE_PIN | MOSI_PIN | CLK_PIN;
    
	//  Pull-up on reset pin.
    set_LCD_RST;
    delay_ms(100);
	clr_LCD_RST;
    delay_ms(100);
    set_LCD_RST;

    //  Enable SPI port: No interrupt, MSBit first, Master mode, CPOL->0, CPHA->0, Clk/4
	SPCR = 0;
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPR1);

    set_LCD_CE;
	
    dSend( 0x21, LCD_CMD );  // LCD Extended Commands.
    dSend( 0xE0, LCD_CMD );  // Set LCD Vop (Contrast).
    dSend( 0x04, LCD_CMD );  // Set Temp coefficent.
    dSend( 0x13, LCD_CMD );  // LCD bias mode 1:48.
    dSend( 0x20, LCD_CMD );  // LCD Standard Commands, Horizontal addressing mode.
    dSend( 0x0C, LCD_CMD );  // LCD in normal mode. (0x0D for Inverted Mode)
    
    LCD.lidx = LCD_CACHE_SIZE - 1;
	LCD.hidx = 0;
} 

/*--------------------------------------------------------------------------------------------------
  Name         :  dSend
  Description  :  Sends data to display controller.
  Argument(s)  :  data -> Data to be sent
                  cd   -> Command or data (see/use enum)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void dSend ( uint8_t data, LcdCmdData cd )
{
    //  Enable display controller (active low).
    clr_LCD_CE;

    if( cd == LCD_DATA)
		set_LCD_DC;
	else if ( cd == LCD_CMD)
		clr_LCD_DC;

    //  Send data to display controller.
    SPDR = data;

    //  Wait until Tx register empty.
    while ( (SPSR & 0x80) != 0x80 );

    //  Disable display controller.
    set_LCD_CE;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  dContrast
  Description  :  Set display contrast.
  Argument(s)  :  contrast -> Contrast value from 0x00 to 0x7F. 
  Return value :  None.
  Notes        :  No change visible at ambient temperature.
--------------------------------------------------------------------------------------------------*/
void dContrast ( uint8_t contrast )
{
    //  LCD Extended Commands.
    dSend( 0x21, LCD_CMD );

    // Set LCD Vop (Contrast).
    dSend( 0x80 | contrast, LCD_CMD );

    //  LCD Standard Commands, horizontal addressing mode.
    dSend( 0x20, LCD_CMD );
}

/*--------------------------------------------------------------------------------------------------
  Name         :  dUpdate
  Description  :  Copies the LCD cache into the device RAM.
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void dRefresh ( void )
{
    int i;
	
// 	printf("\nLCD.lidx = %d \t LCD.idx = %d \t LCD.hidx = %d",LCD.lidx, LCD.idx, LCD.hidx);

	if ( LCD.lidx > LCD.hidx ) return;
	
	/* Error handling */
    if ( LCD.lidx < 0 )
        LCD.lidx = 0;
    if ( LCD.lidx >= LCD_CACHE_SIZE )
        LCD.lidx = LCD_CACHE_SIZE - 1;

    if ( LCD.hidx < 0 )
        LCD.hidx = 0;
    if ( LCD.hidx >= LCD_CACHE_SIZE )
        LCD.hidx = LCD_CACHE_SIZE - 1; 

    dSend( 0x40 | (LCD.lidx / X_RES), LCD_CMD ); // compute and send Y address
    dSend( 0x80 | (LCD.lidx % X_RES), LCD_CMD ); // compute and send X address


    //  Send the buffer.
    for ( i = LCD.lidx; i <= LCD.hidx; i++ )
    {
        dSend( LCD.Cache[i], LCD_DATA );
    }

    //  Reset watermark pointers 
     LCD.lidx = LCD_CACHE_SIZE - 1;
     LCD.hidx = 0;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  dClear
  Description  :  Clears the display. dRefresh must be called next.
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void dClear ( void )
{
    int i;
    for ( i = 0; i < LCD_CACHE_SIZE; i++ )
		LCD.Cache[i] = 0x00;
	
     //  Reset watermark pointers.
    LCD.lidx = 0;
    LCD.hidx = LCD_CACHE_SIZE - 1;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  dPixelIsSet
  Description  :  Check if a pixel is set
  Argument(s)  :  x, y -> Absolute pixel coordinates
  Return value :  1 - true, 0 -false
--------------------------------------------------------------------------------------------------*/
uint8_t dPixelIsSet(uint8_t x, uint8_t y)
{
	if ( x >= X_RES || y>= Y_RES ) return 0;
	return ( LCD.Cache[dCachepos(x,y)] & (1 << (y % 8)) );
}

/*--------------------------------------------------------------------------------------------------
  Name         :  dCachepos
  Description  :  Compute position in cache of a given absolute address in pixels
  Argument(s)  :  x, y -> Absolute pixel coordinates
  Return value :  position in LCD.Cache
--------------------------------------------------------------------------------------------------*/
int16_t dCachepos ( uint8_t x, uint8_t y )
{
	return (y / 8) * X_RES + x; 
}


#ifdef LCD_with_TEXT
/*--------------------------------------------------------------------------------------------------
  Name         :  dCursor
  Description  :  Sets cursor to ( line, x ) location
  Argument(s)  :  line -> [0..5], x -> [0..83]
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void dCursor ( uint8_t line, uint8_t x )
{
	if ( x >= X_RES ) return 0;
	if ( line >= LCD_LINES ) return 0;
	LCD.idx = x + line * X_RES;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  dChar
  Description  :  Displays a character at current cursor location and increment cursor location.
  Argument(s)  :  ch   -> Character to write.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void dChar ( uint8_t ch )
{
	uint8_t i = 0;
	if ( ch<32 && ch > 127 ) return; // check for valid characters
	//if ( LCD.idx >= LCD_CACHE_SIZE) return;  // check if not out of screen
	
	// Updating low watermark index
	if ( LCD.lidx > LCD.idx )
		LCD.lidx = LCD.idx;
	
	while( i < 5 )
	{
		LCD.Cache[LCD.idx++] = pgm_read_byte( & FontTable[ch-32][i]);
		i++; 
	}
	
	LCD.Cache[LCD.idx++] = 0x00; // add space between characters
	
	// Updating high watermark index
	if ( LCD.hidx < LCD.idx )
		LCD.hidx = LCD.idx;	
}

/*--------------------------------------------------------------------------------------------------
  Names        :  dText & dText_FF
  Description  :  Print a '\0' terminated string to Display (from RAM and .pgmspace respectively)
  Argument(s)  :  a string pointer 
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void dText ( uint8_t *string )
{
    while ( *string )
        dChar(*string++ );
}

void dText_FF ( const int8_t *conststr )
{
	char * ptr = conststr;
	while ( pgm_read_byte(ptr) )
		{
			dChar( pgm_read_byte(ptr) );
			ptr++;
		}
}	

#endif

#ifdef LCD_with_GRAPHICS

/*--------------------------------------------------------------------------------------------------
  Name         :  LcdPixel
  Description  :  Displays a pixel at given absolute (x, y) location.
  Argument(s)  :  x, y -> Absolute pixel coordinates
                  mode -> PIXEL_ON, PIXEL_OFF, PIXEL_XOR
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void dPixel( uint8_t x, uint8_t y, LcdPixelMode mode )
{
	/* reject 'out of range' values */
	if ( x >= X_RES || y >= Y_RES ) return; 
	
	LCD.idx = dCachepos(x,y);
	
	switch ( mode ) {
		case PIXEL_ON:  LCD.Cache[LCD.idx] |=  (1 << (y % 8)); break;
		case PIXEL_OFF: LCD.Cache[LCD.idx] &= ~(1 << (y % 8)); break;
		case PIXEL_XOR: LCD.Cache[LCD.idx] ^=  (1 << (y % 8)); break;
		default: return;
	}
	
	if ( LCD.lidx > LCD.idx )
		LCD.lidx = LCD.idx;
	if ( LCD.hidx < LCD.idx )
		LCD.hidx = LCD.idx;	
}

/*--------------------------------------------------------------------------------------------------
  Name         :  dRectangle
  Description  :  Draws a filled or an outlined rectangle (with the width of the border growing inward)
  Argument(s)  :  x1, y1, x2, y2 -> Absolute coordinates of 2 opposite corners
                  pixel_mode -> PIXEL_ON, PIXEL_OFF, PIXEL_XOR
                  linewidth = 0 -> a filled rectangle will be drawn 
                  linewidth - positive values -> an outlined rectangle will be drawn with the line of width provided
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void dRectangle	 ( uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel_mode, uint8_t linewidth)
{
	uint16_t i,j;
	uint8_t newx1,newx2,newy1,newy2;

	// if 1-D rectangle
	if ( x1 == x2 || y1 == y2 ) return; 

	// prepare the Rectangle for drawing 
	newx1 = min(x1,x2);
	newy1 = min(y1,y2);
	newx2 = max(x1,x2);
	newy2 = max(y1,y2);
	
	x1 = newx1;
	x2 = newx2;
	y1 = newy1;
	y2 = newy2;
	
	// draw rectangle pixel by pixel (!! should improve this; in certain situations it can be drawn by filling octets in cache)
	for ( j=x1; j<=x2; j++ )
		for ( i=y1; i<=y2; i++ )
			dPixel(j,i,pixel_mode);

	// if a linewidth is provided and it's a valid one
	if ( linewidth > 0 && ( linewidth <= min( x2-x1, y2-y1 )/2 ))
		// draw an inner rectangle using PIXEL_XOR mode to extrude from the outer one.
		dRectangle( x1 + linewidth, y1 + linewidth, x2 - linewidth, y2 - linewidth, PIXEL_XOR, 0 );	
}

// Dummy char comparision function
int8_t max(int8_t el1, int8_t el2)
{
	if(el1>el2)
		return el1;
	else 
		return el2;
}

// Dummy char comparision function
int8_t min(int8_t el1, int8_t el2)
{
	if(el1<el2)
		return el1;
	else 
		return el2;
}


/*--------------------------------------------------------------------------------------------------
  Name         :  dLine
  Description  :  Draws a line between two points on the display using Jack Bresenham algorithm.
  Argument(s)  :  x1, y1 -> Absolute pixel coordinates for line origin.
                  x2, y2 -> Absolute pixel coordinates for line end.
                  pixel_mode -> PIXEL_ON, PIXEL_OFF, PIXEL_XOR
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void dLine       ( uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel_mode , uint8_t linewidth )
{
	int16_t stepx, stepy, fraction;
	// error handling
	if ( x1 >= X_RES || y1 >= Y_RES || x2 >= X_RES || y2 >= Y_RES ) return; 
	
	int16_t dy = y2-y1;
	int16_t dx = x2-x1;

	if ( dx < 0 )
	{	
		dx = -dx;	
		stepx = -1;
	}
	else
	{	stepx = 1;	}
	
	if ( dy < 0 )
	{
		dy = -dy;
		stepy = -1;
	}
	else
	{	stepy = 1;	}
	
	dx <<= 1;
    dy <<= 1;
	
    dPixel( x1, y1, pixel_mode );

    if ( dx > dy )
    {
        fraction = dy - dx/2;
        while ( x1 != x2 )
        {
            if ( fraction >= 0 )
            {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;
            dPixel( x1, y1, pixel_mode );
        }
    }
    else
    {
        fraction = dx - dy/2;
        while ( y1 != y2 )
        {
            if ( fraction >= 0 )
            {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
            dPixel( x1, y1, pixel_mode );
        }
    }
}

#endif
/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/

