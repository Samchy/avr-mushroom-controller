#include "timer1.h"

/**--------------------------------------------------------------------------------------------------
  Name         :  initTimer1
  Description  :  initializations of Timer1
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------**/
void initTimer1()
{
	TCCR1A = 0;
	TCCR1B = 0;

	#if T1_PRESCALER == 1
		TCCR1B |= (1<<CS10); 
	#elif T1_PRESCALER == 8
		TCCR1B |= (1<<CS11); 
	#elif T1_PRESCALER == 64
		TCCR1B |= (1<<CS11)|(1<<CS10);
	#elif T1_PRESCALER == 256
		TCCR1B |= (1<<CS12);
	#elif T1_PRESCALER == 1024
		TCCR1B |= (1<<CS12)|(1<<CS10); 
	#else
		#error Invalid value for T1_PRESCALER. Must be 1,8,64,256 or 1024.
	#endif
}

/**--------------------------------------------------------------------------------------------------
  Name         :  tk2ms
  Description  :  Function to convert ticks to miliseconds
  Argument(s)  :  ticks
  Return value :  miliseconds
--------------------------------------------------------------------------------------------------**/
float tk2ms(time_t ticks)
{ 
    return ( (float)(ticks) * T1_PRESCALER * 1e3 / (F_CPU));
}

/**--------------------------------------------------------------------------------------------------
  Name         :  tk2us
  Description  :  Function to convert ticks to microseconds
  Argument(s)  :  ticks
  Return value :  microseconds
--------------------------------------------------------------------------------------------------**/
float tk2us(time_t ticks)
{
	return ( (float)ticks * T1_PRESCALER * 1e3 / F_CPU * 1e3);
}

/**--------------------------------------------------------------------------------------------------
  Name         :  ms2tk
  Description  :  Function to convert miliseconds to ticks. 
  Argument(s)  :  miliseconds
  Return value :  ticks
--------------------------------------------------------------------------------------------------**/
time_t ms2tk(float ms)
{															
 	return (time_t) ( ms * F_CPU / 1e3 / (T1_PRESCALER));															
// 	alternative for the above line	( us2tk( ((float)ms-(int32_t)ms) * 1e3) ) 
}

/**--------------------------------------------------------------------------------------------------
  Name         :  us2tk
  Description  :  Function to convert microseconds to ticks. 
  Argument(s)  :  microseconds
  Return value :  ticks
--------------------------------------------------------------------------------------------------**/
time_t us2tk(float us)
{															
 	return (time_t) \
 	( \
		( us * F_CPU / 1e6 / (T1_PRESCALER) ) + \
		( (us - (int32_t)us)* ((float)F_CPU / 1e6 / T1_PRESCALER) ) \
	);																
}

/**--------------------------------------------------------------------------------------------------
  Name         :  clock
  Description  :  Get current timer/counter TCNT value.
  Argument(s)  :  None.
  Return value :  ticks
--------------------------------------------------------------------------------------------------**/
time_t clock()
{
	/* Clearing global interrupts while reading TCNT1 since it's an atomic operation */
	cli(); 
	time_t tcntvalue = TCNT1; 
	sei();
	return tcntvalue;
}

/**--------------------------------------------------------------------------------------------------
  Name         :  difftime_tk
  Description  :  Returns difference in ticks  between time1 and time2.
  Argument(s)  :  None.
  Return value :  ticks
--------------------------------------------------------------------------------------------------**/
time_t difftime_tk(time_t time2, time_t time1) 
{
	return (time_t) ( ((uint32_t)time2 - time1 + T1_OVF_VALUE + 1) % (T1_OVF_VALUE + 1) );
}

/**--------------------------------------------------------------------------------------------------
  Name         :  difftime_us
  Description  :  Returns difference in microseconds  between time1 and time2.
  Argument(s)  :  None.
  Return value :  microseconds
--------------------------------------------------------------------------------------------------**/
float difftime_us(time_t time2, time_t time1) 
{
	return (float) (tk2us(difftime_tk(time2,time1)));
}

/**--------------------------------------------------------------------------------------------------
  Name         :  difftime_ms
  Description  :  Returns difference in miliseconds  between time1 and time2.
  Argument(s)  :  None.
  Return value :  miliseconds
--------------------------------------------------------------------------------------------------**/
float difftime_ms(time_t time2, time_t time1) 
{ 
	return (float) tk2ms(difftime_tk(time2,time1));
}

/**--------------------------------------------------------------------------------------------------
  Name         :  basic_delay_us
  Description  :  Delays a number of microseconds
  Argument(s)  :  microseconds
  Return value :  None.
--------------------------------------------------------------------------------------------------**/
void delay_us(float us)
{ 
	cli();  // Clear interrupts to have an accurate delay
	
	time_t start=TCNT1; // read current
 	float calibration = 4.241; // Empirical calibration factor
	time_t stop = start + us2tk(us - calibration);
	
	if(stop<start) // If an overflow will occur in order to reach stop value
		while(TCNT1>start); // loop until overflow
	while(TCNT1<=stop); // loop until stop reached
	
	sei(); 
}

/**--------------------------------------------------------------------------------------------------
  Name         :  basic_delay_ms
  Description  :  Delays a number of miliseconds using delay_us(). It's very limited due to timer/counter resolution.
  Argument(s)  :  miliseconds
  Return value :  None.
--------------------------------------------------------------------------------------------------**/
void basic_delay_ms(float ms) 
{ 
	delay_us(ms*1e3);
}

/**--------------------------------------------------------------------------------------------------
  Name         :  delay_ms
  Description  :  Delays a number of miliseconds using basic_delay_ms().
  Argument(s)  :  miliseconds
  Return value :  None.
--------------------------------------------------------------------------------------------------**/
void delay_ms(float ms)
{
	int32_t i;
	for( i=(int32_t)ms; i>0; i--)
		basic_delay_ms(1);
	delay_us(ms - (int32_t)ms);
}
