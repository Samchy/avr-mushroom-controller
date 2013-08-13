//for debugging
#include <stdio.h>

#include <stdint.h>
#include <avr/io.h>

#define DC_GAIN (8.5)   // DC gain of the amplifier


/***********************Software Related Macros************************************/
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interval(in milisecond) between each samples in 
                                                     //normal operation

/**********************Application Related Macros**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
#define         ZERO_POINT_VOLTAGE           (0.220) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define         REACTION_VOLTGAE             (0.020) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2

/*****************************Globals***********************************************/
float CO2Curve[3]  =  { 2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTGAE/(2.602-3)) };   

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

uint16_t readMG811(void)
{
	uint16_t retval = 0;
	uint8_t temp;
	ADCSRA |= (1<<ADSC); 
	while( ADCSRA & (1<<ADSC) )
		;
	retval = ADCL;
	//printf("\nADCL: %u", retval);
	temp = ADCH;
	//printf("\nADCH: %u", temp);
	retval |= ((uint16_t)temp<<8);

	return retval;
}

/*
void loop()
{
    int percentage;
    float volts;
    
   
    volts = MGRead(MG_PIN);
    Serial.print( "SEN-00007:" );
    Serial.print(volts); 
    Serial.print( "V           " );
    
    percentage = MGGetPercentage(volts,CO2Curve);
    Serial.print("CO2:");
    if (percentage == -1) {
        Serial.print( "<400" );
    } else {
        Serial.print(percentage);
    }
 
    Serial.print( "ppm" );  
    Serial.print("\n");
    
    if (digitalRead(BOOL_PIN) ){
        Serial.print( "=====BOOL is HIGH======" );
    } else {
        Serial.print( "=====BOOL is LOW======" );
    }
      
    Serial.print("\n");
    
    delay(200);
}



*****************************  MGRead *********************************************
Input:   mg_pin - analog channel
Output:  output of SEN-000007
Remarks: This function reads the output of SEN-000007
************************************************************************************
float MGRead(int mg_pin)
{
    int i;
    float v=0;

    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        v += readMG811();
        delay(READ_SAMPLE_INTERVAL);
    }
    v = (v/READ_SAMPLE_TIMES) *5/1024 ;
    return v;  
}

*****************************  MQGetPercentage **********************************
Input:   volts   - SEN-000007 output measured in volts
         pcurve  - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(MG-811 output) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************
int  MGGetPercentage(float volts, float *pcurve)
{
   if ((volts/DC_GAIN )>=ZERO_POINT_VOLTAGE) {
      return -1;
   } else { 
      return pow(10, ((volts/DC_GAIN)-pcurve[1])/pcurve[2]+pcurve[0]);
   }
}
*/

