avr-mushroom-controller
=======================

Air humidity and CO2 level controller using an ATmega168, two relays, a remote-control(Philips RC5) and a Nokia 5110 monochrome LCD.

Resources used:
	Timer 1 - always counts from 0x0 to 0xFFFF. It is accessed by the timer1.h measureing and delay functions.
		Timer1 COMPB IRQ: used to generate a 1ms pause at the beggining of the communication with DHT22
		Timer1 COMPA IRQ: used in the rc5 decoding state machine. If an error is detected, COMPA is used to generate a pause in 						  the state machine until all the remaining bits arrive (of course, they are ignored). 
		Timer1 ICP IRQ: used to implement the DHT22 decoding state machine.

	Timer 0 
		Timer 0 OVF IRQ: used to modify a flag every second
