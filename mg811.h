#ifndef MG811_H
#define MG811_H

#include <stdint.h>

#define DC_GAIN (8.5)   // DC gain of the amplifier

/* Software Related Macros */
#define		READ_SAMPLE_INTERVAL		(10)    // number of samples
#define		READ_SAMPLE_TIMES			(1)     // time interval between samples

/* Application Related Macros */
/* These two values differ from sensor to sensor. User should derermine this value. */
#define		ZERO_POINT_VOLTAGE		(0.220) 	// sensor output in volts when the concentration of CO2 is 400PPM
#define		REACTION_VOLTGAE		(0.020) 	// voltage drop of the sensor when moving it into 1000ppm medium

/* Gas characteristic */
extern float CO2Curve[3]; 

void initMG811(void);
float MG811_ReadVolts(void);
uint32_t MG811_ReadPPM(float volts, float * pcurve);

#endif
