// #include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
// #include <avr/pgmspace.h>
#include "utils.h"
#include "timer1.h"
#include "rc5.h"
#include "fifo.h"
#include "lph7366.h"
#include "dht22.h"
#include "backlight.h"
#include "timer0.h"
#include "mg811.h"
#include "uart.h"

#define OFF		0
#define UART_DEBUG	1	
#define LCD_DEBUG	2
#define DEBUG		UART_DEBUG


extern fifoType rc5buffer;
void checkIR(void);
void rc5store(uint16_t);

static FILE uartstream = FDEV_SETUP_STREAM(uartSendChar, uartGet, _FDEV_SETUP_RW);

int main(void)
{
	uint8_t string[10];
	uint8_t i=0;
	
	// configure printf, scanf etc. for USART
	stdout = stdin = &uartstream; 

	// Relays
	sbi(DDRD,6); sbi(PORTD,6);
	sbi(DDRD,7); sbi(PORTD,7);

	// initial state of the ICP
	cbi(DHT22_DIR, DHT22_PIN);
	sbi(DHT22_PORT, DHT22_PIN);

	initMG811();

	initTimer0();
	initTimer1();
	rc5init(rc5store, RC5_INVERTED); // Enable user control

	// LCD related
#if (DEBUG != UART_DEBUG)
	initBacklight();
	initLCD();
	dContrast(0x35);
	
	dClear();
	dCursor(0,0);
	dText("Reset");
	dRefresh();
#else
	initUART();
#endif

	// Enable interrupts in SREG.I
	sei();

	while(1)
	{
		checkIR();
		
		if( DHT22_State() == DHT22_READY )
		{
			//sprintf(string, "T: %.1f", DHT22_ReadTemperature());
			//dCursor(2,0);
			//dText(string);
			//sprintf(string, "H: %.1f", DHT22_ReadHumidity());
			//dCursor(3,0);
			//dText(string);
			//dRefresh();
			printf("\nT: %.1f", DHT22_ReadTemperature());
			printf("\nH: %.1f", DHT22_ReadHumidity());
		}

		if(OneSecondFlag)
		{

			DHT22_Read();
			uint16_t mg811val = readMG811();
			printf("\nCO2: %u", mg811val);
			//i+=10;
			//dContrast(i);
			//sprintf(string, "CO2: %u", readMG811());
			//dCursor(1,0);
			//dText(string);
			//dRefresh();

			OneSecondFlag = 0;
		}
	}

	return 1;
}

/**--------------------------------------------------------------------------------------------------
  Description  :  Store the command to the rc5buffer circular buffer and filter repetitive commands.
				  This function is called by the rc5 library after successfull decoding.
--------------------------------------------------------------------------------------------------**/
// 
void rc5store(uint16_t data)
{
	uint8_t command = rc5comm(data);
	static uint8_t prevtogglebit;
	
	// exceptions of buttons who can be continuously pressed
	if(command != CHUP && command != CHDOWN)
	{
		// Take care of long press of a button
		uint8_t currtogglebit = rc5tog(data);
	
		if(prevtogglebit == currtogglebit)
			return;
		prevtogglebit = currtogglebit;	
	}

	fifoWrite(&rc5buffer, (int8_t)command);	
}

/**--------------------------------------------------------------------------------------------------
  Description  :  Called in the main loop. Checks the FIFO for new commands and executes corresponding 
				  actions.
--------------------------------------------------------------------------------------------------**/
void checkIR(void)
{
	uint8_t command;

	if( !fifoIsEmpty(&rc5buffer) )
	{
		fifoRead(&rc5buffer, (int8_t *)&command);
#if (DEBUG == LCD_DEBUG)
		dCursor(0,0);
		char debugstr[6];
		sprintf(debugstr, "C:%d", command);
		dText(debugstr);
		dRefresh();
#endif
		switch (command) {
			case CHUP: 
				incrBacklight(); 
			break;

			case CHDOWN: 
				decrBacklight(); 
			break;

			case POWER: 
				if( getBacklight() ) 
					setBacklight(0);
				else
					setBacklight(7);
			break;

			case 2: tbi(PORTD,6); break;
			case 3: tbi(PORTD,7); break;
			case 1: DHT22_Read(); break;	 
		}

		
	}
}



