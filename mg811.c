/**------------------------------------------------------------------------------------------------
  Note : 	This file actually reads and processes the output of a module named SEN-000007 built 
			around the MG-811 sensor so not the sensor itself. Credits go to these people:
				Tiequan Shao: tiequan.shao@sandboxelectronics.com
        		Peng Wei:     peng.wei@sandboxelectronics.com
			I have only adapted their code to meet the requirements of avr-mushroom-controller 
			project on my github https://github.com/misomosi-biz/avr-mushroom-controller.
-------------------------------------------------------------------------------------------------**/
#include <stdio.h>

#include <stdint.h>
#include <avr/io.h> 
#include <math.h>
#include "mg811.h"

float CO2Curve[3]  =  { 2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTGAE / ( 2.602 - 3 )) };

/**------------------------------------------------------------------------------------------------
  Description : 	Initialize ADC
-------------------------------------------------------------------------------------------------**/
void initMG811()
{
	// disable digital function
	DIDR0 = 0xff;
	// AVCC reference, channel 0
	ADMUX =  (1<<REFS0) ;
	// 128 prescaler => 125 kHz
	ADCSRA = (1<<ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
	// Free running mode
	ADCSRB = 0;
	
	// dummy read
	ADCSRA |= (1<<ADSC); 
}

/**------------------------------------------------------------------------------------------------
  Description 	: 	Reads ADC channel 0
  Return		: 	0-1024
-------------------------------------------------------------------------------------------------**/
static uint16_t MG811_ReadADC(void)
{
	uint16_t retval = 0;
	ADCSRA |= (1<<ADSC); 
	while( ADCSRA & (1<<ADSC) )
		;
	retval = ADCL;
	retval |= ((uint16_t)ADCH<<8);

	return retval;
}

/**------------------------------------------------------------------------------------------------
  Description 	: 	reads samples and outputs an average voltage
  Return		: 	voltage as float
-------------------------------------------------------------------------------------------------**/
float MG811_ReadVolts(void)
{
	int i;
	float v=0;

	for ( i=0; i<READ_SAMPLE_TIMES; i++ ) 
	{
		v += MG811_ReadADC();
		delay_ms(READ_SAMPLE_INTERVAL);
    }
	v = ( v / READ_SAMPLE_TIMES ) * 3.3 / 1024 ;

	return v;  
}

/**-------------------------------------------------------------------------------------------------
  Description 	: 	By using the slope and a point of the line. The x(logarithmic value of ppm) 
         			of the line could be derived if y (MG-811 output) is provided. As it is a 
         			logarithmic coordinate, power of 10 is used to convert the result to 
					non-logarithmic value.
  Arguments		:	volts - ADC measuring in volts
					pcurve - pointer to the curve of the CO2
  Return		: 	ppm 
-------------------------------------------------------------------------------------------------**/
uint32_t MG811_ReadPPM(float volts, float * pcurve)
{
	if ( ( volts / DC_GAIN ) >= ZERO_POINT_VOLTAGE ) 
		return -1;
	else 
		return pow( 10, ( ( volts / DC_GAIN )- pcurve[1] ) / pcurve[2] + pcurve[0] );
}
