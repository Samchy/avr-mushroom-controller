#ifndef _DHT22_H
#define _DHT22_H

#include <stdint.h>
#include <avr/io.h>

#define DHT22_DIR	DDRB
#define DHT22_PORT	PORTB
#define DHT22_PIN	0

// Tolerance of measurements in microseconds
#define DHT22_TOLERANCE	20

typedef struct {
	float Temperature;
	float Humidity;
} DHT22_Info_Type;

typedef enum {
	DHT22_IDLE,
	DHT22_BUSY,
	DHT22_READY
} DHT22_STATE_Type;

extern DHT22_Info_Type DHT22_Info;

void DHT22_Read(void);
float DHT22_ReadTemperature(void);
float DHT22_ReadHumidity(void);
DHT22_STATE_Type DHT22_State(void);

#endif
