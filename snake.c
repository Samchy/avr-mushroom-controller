#include "snake.h"

static uint8_t chc(uint8_t * , int8_t offset, uint8_t overflowvalue);
static void PlaceFood(void);
static void ComputeHC(Snake_t * snake);

void initGame()
{
	dClear();
	dBorder();
	initSnake();
	PlaceFood();
}

void initSnake()
{	
	Snake.speed = 7;
	Snake.hx = 12;
	Snake.tx = 4;
	Snake.hy = 3;
	Snake.ty = 3;
	Snake.hd = RIGHT;
	Snake.nhd = RIGHT;
	Snake.td = RIGHT;
	dSnakeElement( Snake.hx, Snake.hy, RIGHT, PIXEL_ON );
	dSnakeElement( Snake.tx+SNKSTEP, Snake.ty, RIGHT, PIXEL_ON );
	dSnakeElement( Snake.tx, Snake.ty, RIGHT, PIXEL_ON );
}

void dBorder()
{
	dLine(0,0,X_RES-1,0,PIXEL_ON,1);
	dLine(X_RES-1,0,X_RES-1,Y_RES-1,PIXEL_ON,1);
	dLine(0,Y_RES-1,X_RES-1,Y_RES-1,PIXEL_ON,1);
	dLine(0,0,0,Y_RES-2,PIXEL_ON,1);
}

void dSnakeElement(uint8_t x, uint8_t y, uint8_t orientation, uint8_t pixel_mode)
{	
		/*
		A snake element: 
		
		    ^ y
		    |
		    |
		o o o o
		o o X o - - - > x
		o o o o
		
		*/
	switch (orientation) {
		case RIGHT: dRectangle(x-2, y-1, x+1, y+1, pixel_mode, 0 ); break;
		case LEFT : dRectangle(x-1, y-1, x+2, y+1, pixel_mode, 0 ); break;
		case UP	  : dRectangle(x-1, y-1, x+1, y+2, pixel_mode, 0 ); break;
		case DOWN : dRectangle(x-1, y-2, x+1, y+1, pixel_mode, 0 ); break;
	}
}


void dSnake(Snake_t * snake)
{
	uint8_t eat = 0, bang = 0;

	ComputeHC(snake);
	
	// graphically check if there is something in the new computed position
	if ( dPixelIsSet(snake->hx, snake->hy) )
			bang = 1; // mark that there is
	
	// if we're on food position
	if ( snake->hx == snake->fx && snake->hy == snake->fy)
			eat = 1;
	
	// if not on food position and bang happened
	if ( bang && !eat )
	{
		TIMSK1 &= ~(1<<OCIE1A);
		TIMSK1 &= ~(1<<ICIE1); // Timer1 Input Capture Interrupt Enable
		delay_ms(2000);
		initGame();
		TIMSK1 = (1<<OCIE1A);
		TIMSK1 |= (1<<ICIE1); // Timer1 Input Capture Interrupt Enable
		return 0;
	}
	// draw new head
	dSnakeElement(snake->hx, snake->hy, snake->nhd, PIXEL_ON);
	// update head direction
	snake->hd = snake->nhd;
	
	if (!eat) 
	{
	// delete tail if no food eaten
	dSnakeElement(snake->tx,snake->ty,snake->td,PIXEL_OFF);

	// switch tail direction by graphically checking continuity
	if ( dPixelIsSet( snake->tx + 2 , snake->ty))  snake->td = RIGHT;  // look right
	if ( dPixelIsSet( snake->tx, snake->ty - 2 ))  snake->td = UP; 	 // look up
	if ( dPixelIsSet( snake->tx - 2 , snake->ty))  snake->td = LEFT;  // look left
	if ( dPixelIsSet( snake->tx, snake->ty + 2 ))  snake->td = DOWN;  // look down
	
	// switch tail position
	switch (snake->td) {
		case UP: 
			chc(&snake->ty,-SNKSTEP, Y_RES);
			break;
		case DOWN:
			chc(&snake->ty, SNKSTEP, Y_RES);
			break;
		case LEFT:
			chc(&snake->tx,-SNKSTEP, X_RES);
			break;
		case RIGHT:
			chc(&snake->tx, SNKSTEP, X_RES);
			break;
	}
	}
	
	else PlaceFood();
	
}

/** Compute head coordinates **/
void ComputeHC(Snake_t * snake)
{
		// switch head direction
	switch (snake->hd) {
		case UP: 
			if (snake->nhd == UP)	  chc(&snake->hy,-4, Y_RES);
			if (snake->nhd == RIGHT)  chc(&snake->hx, 4, X_RES); 
			if (snake->nhd == LEFT)   chc(&snake->hx,-4, X_RES); 
			break;
		case DOWN:
			if (snake->nhd == DOWN)   chc(&snake->hy, 4, Y_RES);
			if (snake->nhd == RIGHT)  chc(&snake->hx, 4, X_RES); 
			if (snake->nhd == LEFT)   chc(&snake->hx,-4, X_RES); 
			break;
		case LEFT:
			if (snake->nhd == LEFT)   chc(&snake->hx,-4, X_RES);
			if (snake->nhd == UP) 	  chc(&snake->hy,-4, Y_RES); 
			if (snake->nhd == DOWN)   chc(&snake->hy, 4, Y_RES); 
			break;
		case RIGHT:
			if (snake->nhd == RIGHT)  chc(&snake->hx, 4, X_RES);
			if (snake->nhd == UP) 	  chc(&snake->hy,-4, Y_RES); 
			if (snake->nhd == DOWN)   chc(&snake->hy, 4, Y_RES); 
			break;
	}
}

/** change coordinate **/
uint8_t chc(uint8_t * v, int8_t offset, uint8_t overflow)
{
	return *v = (uint8_t)(((int8_t)overflow + (int8_t)*v + offset) % (int8_t)overflow);
}

void PlaceFood()
{
	cli();
	srand(17);
	do
	{
		Snake.fx = 4 * (rand() % (X_RES/4));
		Snake.fy = 4 * (rand() % (Y_RES/4)) -1;
	}
	while(dPixelIsSet(Snake.fx,Snake.fy));
	
	// improvising by using the snake element draw function to draw the food	
	dSnakeElement(Snake.fx,Snake.fy,RIGHT,PIXEL_ON); 
	// turn off unwanted pixels
	dPixel(Snake.fx-2,Snake.fy,PIXEL_OFF); 
	dPixel(Snake.fx-2,Snake.fy-1,PIXEL_OFF);
	dPixel(Snake.fx-2,Snake.fy+1,PIXEL_OFF);	
	sei();
}

