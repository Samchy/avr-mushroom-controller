#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <stdint.h>

void initBacklight(void);
void setBacklight(uint8_t val);
uint8_t getBacklight(void);
void incrBacklight(void);
void decrBacklight(void);

#endif
