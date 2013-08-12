#ifndef TIMER0_H
#define TIMER0_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void initTimer0(void);

extern uint8_t OneSecondFlag;

#endif
