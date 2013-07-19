#include "Main.h"

extern cBufferType rc5buffer;
void CheckIR(void);

void rc5store(uint16_t);

//  static FILE usartstream = FDEV_SETUP_STREAM(usartSendChar, usartGet,_FDEV_SETUP_RW);

int main(void)
{
//  stdout = stdin = &usartstream; // configure streams

	// Relays
	sbi(DDRD,6); sbi(PORTD,6);
	sbi(DDRD,7); sbi(PORTD,7);

	//  initUSART();
	initTimer1();
	rc5init(rc5store, RC5_INVERTED); // Enable user control
	initLCD();
	dContrast(0x40); // 0x25 a mers prima oară // 0x40 a mers după adăugarea rezistorilor 

	dClear();
	dCursor(1,0);
	dText("Hello");
	dRefresh();

	sei();
	 initBacklight();

	while(1)
	{
		CheckIR();
		if( DHT22_State() == DHT22_READY )
		{
			uint8_t string[10];
			sprintf(string, "%.1f",DHT22_ReadTemperature());
			dCursor(3,0);
			dText(string);
			sprintf(string, "%.1f", DHT22_ReadHumidity());
			dCursor(3,0);
			dText(string);

			dRefresh();
		}
	}
	return 0;
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
		cBufferWrite(&rc5buffer, (int8_t)command);	
}

/**--------------------------------------------------------------------------------------------------
  Description  :  Called in the main loop. Checks the FIFO for new commands and executes corresponding 
				  actions.
--------------------------------------------------------------------------------------------------**/
void CheckIR(void)
{
	uint8_t command;

	if( !cBufferIsEmpty(&rc5buffer) )
	{
		cBufferRead(&rc5buffer, (int8_t *)&command);
		dClear();
		dCursor(0,0);
		char debugstr[4];
		sprintf(debugstr, "%d", command);
		dText(debugstr);

		switch (command) {
			case CHUP: incrBacklight(); break;
			case CHDOWN: decrBacklight(); break;
			case POWER: 
						if( getBacklight() ) 
							setBacklight(0);
						else
							setBacklight(7);
			break;
			case 6: tbi(PORTD,6); break;
			case 7: tbi(PORTD,7); break;
			case 9: DHT22_Read(); break;
	 
		}
		dCursor(5,0);				
		sprintf(debugstr,"%d",getBacklight()); 
		dText(debugstr);
		dRefresh();
	}
}



