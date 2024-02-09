#include <util/delay.h>
#include <stddef.h>

#include "twi.h"

#define TWI_FREQ 100000 // Hz

static inline void twi_delay(void) {
    _delay_us(1000000.0f / (TWI_FREQ * 2));
}

static inline void release_sda(void) {
    PRT |= (1 << SDA);
}

static inline void push_sda(void) {
    PRT &= ~(1 << SDA);
}

static inline bool read_sda(void) {
    return ((PIN & (1 << SDA)) != 0);
}

static inline void release_scl(void) {
    PRT |= (1 << SCL);
}

static inline void push_scl(void) {
    PRT &= ~(1 << SCL);
}

static inline bool read_scl(void) {
    return ((PIN & (1 << SCL)) != 0);
}

static void wait_scl(void) {
    while(!read_scl()); // TODO: add timeout
}

static inline void push_all(void) {
    PRT &= ~(1 << SCL) & ~(1 << SDA);
}

/***************** INTERFACE *****************/

void twi_init_master(void) {
    PRT |= (1 << SCL) | (1 << SDA);
    DDR |= (1 << SCL) | (1 << SDA);
  
    USIDR = 0xff;
    USICR = (1 << USIWM1) |
            (1 << USICS1) |
            (1 << USICLK);
    USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);
  
    twi_delay();
}

void twi_start(void) {
    push_sda();
    twi_delay();
    push_scl();
    USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);
}

void twi_stop(void) {
    push_sda();
    twi_delay();
    release_scl();
    wait_scl();
    twi_delay();
    release_sda();
    twi_delay();
}

bool twi_write_byte(const uint8_t data) {
    uint8_t res;
  
    USIDR = data;
    release_sda(); // SDA = USIDR[7]
    USISR &= 0b11110000;
    do {
        twi_delay();
        USICR |= (1 << USITC); // SCL = 1
        twi_delay();
        USICR |= (1 << USITC); // SCL = 0; USIDR = USIDR << 1
    } while((USISR & (1 << USIOIF)) == 0);
    USIDR = 0xff; // SDA = 1(1111111)
  
    twi_delay();
    release_scl();
    wait_scl();
    twi_delay();
    res = !read_sda();
    push_all();
  
    return res;
}

bool twi_write_bytes(const uint8_t *const data, const uint8_t size, const twi_write_delay_callback_t delay_callback) {
    for(uint8_t i = 0; i < size; ++i) {
        if(twi_write_byte(*((uint8_t *) data + i))) {
            if(delay_callback != NULL) {
                delay_callback(i, size);
            }
        } else {
            return false;
        }
    }

    return true;
}

bool twi_write_begin(const uint8_t address) {
    twi_start();
  
    return twi_write_byte(address);
}

void twi_write_end(void) {
    twi_stop();
}
