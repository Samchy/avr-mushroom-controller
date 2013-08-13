avr-mushroom-controller
=======================

Air humidity and CO2 level controller using an ATmega168, two relays, a remote-control(Philips RC5) and a Nokia 5110 monochrome LCD.

Resources used:
	GPIO & External IRQ
		PORTD.6 - Water pump relay (to increase humidity)
		PORTD.7 - Air vent relay (to decrease CO2 level) 
		PORTD.3 - Remote controller sensor. 
				- INT1 implements the state machine
		PORTB.0 - DHT22 pin		
		PORTB.1 - Backlight
		
		PORTD.0 - LCD Reset (RST)
		PORTD.1 - LCD Chip Enable (CE)
		PORTD.2 - LCD Data/Command (DC)
		PORTB.3 - LCD MOSI
		PORTB.5 - LCD SCK

	Timer 0 
		Timer 0 OVF IRQ: used to modify a flag every second
	Timer 1 
		Always counts from 0x0 to 0xFFFF. It is accessed by the timer1.c/h measureing and delay functions.
		Timer1 COMPB IRQ: used to generate a 1ms pause at the beggining of the communication with DHT22
		Timer1 COMPA IRQ: used in the rc5 decoding state machine. If an error is detected, COMPA is used to generate a pause in 						  the state machine until all the remaining bits arrive (these bits are ignored). 
		Timer1 ICP IRQ: used to implement the DHT22 decoding state machine.
	Timer 2
		Used to generate PWM for the backlight by adjusting COMPA value.
		TIMER2 COMPA IRQ: outputs 0 on the pin
		TIMER2 OVF IRQ: outputs 1 on the pin
		
	ADC
		Channel 0 - MG811 reading

	SPI
		Interfacing with Nokia LCD

	UART
		Debugging

	

	
	
