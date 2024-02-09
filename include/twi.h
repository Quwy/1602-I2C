#ifndef __TWI_H__
#define __TWI_H__

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#define PRT PORTB
#define DDR DDRB
#define PIN PINB
#define SDA PINB0
#define SCL PINB2

typedef void (*twi_write_delay_callback_t)(const uint8_t, const uint8_t);

void twi_init_master(void);
void twi_start(void);
void twi_stop(void);
bool twi_write_byte(const uint8_t data);
bool twi_write_bytes(const uint8_t *const data, const uint8_t size, const twi_write_delay_callback_t delay_callback);
bool twi_write_begin(const uint8_t address);
void twi_write_end(void);

#endif /* __TWI_H__ */
