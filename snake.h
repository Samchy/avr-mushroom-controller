#ifndef _SNAKE_H
#define _SNAKE_H

#include <stdlib.h>
#include "Main.h"
#include "utils.h"

#include "rc5.h"
#include "lph7366.h"

#define SNKSTEP 4

typedef struct {
	uint8_t hx; // head x coordinate
	uint8_t hy; // head y coordinate
	uint8_t hd; // head direction
	uint8_t nhd; // next head direction
	
	uint8_t tx; // tail x coordinate
	uint8_t ty; // tail y coordinate
	uint8_t td; // tail direction
	
	uint8_t fx; // food x coordinate
	uint8_t fy; // food y coordinate
	
	uint8_t speed; // the speed is inverted (check the formula in TIMER1_COMPA_vect)
	
} Snake_t;

volatile Snake_t Snake;

void initGame(void);
void initSnake( void );
void dSnakeElement(uint8_t x, uint8_t y, uint8_t orientation, uint8_t pixel_mode);
void dSnake( Snake_t * snake );
void dBorder( void );

#endif
