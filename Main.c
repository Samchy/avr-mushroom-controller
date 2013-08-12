#include "Main.h"
#include "backlight.h"
#include <util/delay.h>

extern fifoType rc5buffer;
void CheckIR(void);
void rc5store(uint16_t);
void timertest(void);

//  static FILE usartstream = FDEV_SETUP_STREAM(usartSendChar, usartGet,_FDEV_SETUP_RW);

int main(void)
{
//  stdout = stdin = &usartstream; // configure streams

	// Relays
	sbi(DDRD,6); sbi(PORTD,6);
	sbi(DDRD,7); sbi(PORTD,7);

	// initial state of the ICP
	cbi(DHT22_DIR, DHT22_PIN);
	sbi(DHT22_PORT, DHT22_PIN);

	//  initUSART();
	initTimer1();
	rc5init(rc5store, RC5_INVERTED); // Enable user control
	initLCD();
	dContrast(0xBF);

	dClear();
	dCursor(0,0);
	dText("Reset");
	dRefresh();

	sei();
	initBacklight();

	while(1)
	{
		CheckIR();

		if( DHT22_State() == DHT22_READY )
		{
			uint8_t string[10];
			sprintf(string, "T: %.1f", DHT22_ReadTemperature());
			dCursor(2,0);
			dText(string);
			sprintf(string, "H: %.1f", DHT22_ReadHumidity());
			dCursor(3,0);
			dText(string);

			dRefresh();
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
void CheckIR(void)
{
	uint8_t command;

	if( !fifoIsEmpty(&rc5buffer) )
	{
		fifoRead(&rc5buffer, (int8_t *)&command);
		dClear();
		dCursor(0,0);
		char debugstr[6];
		sprintf(debugstr, "C:%d", command);
		dText(debugstr);

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

		dRefresh();
	}
}

void timertest()
{	uint8_t errorstr[10];
	uint16_t time1, time2, difftk;
	float diffus, diffms;

	time1 = clock();	
	_delay_us(70);
	time2 = clock();

	difftk = difftime_tk(time2, time1);
	diffus = tk2us(difftk);

	dCursor(3,0); 
	sprintf(errorstr, "difftk:%u", difftk);	
	dText(errorstr);

	dCursor(5,0); 
	sprintf(errorstr, "diffus:%.1f", diffus);	
	dText(errorstr);

	dRefresh();

}



