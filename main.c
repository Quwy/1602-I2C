/*
 * main.c
 *
 * Created: 08.02.2024 18:59:34
 * Author : Mike
 */ 

#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>

#include "lcd.h"

int main(void) {
    const char STR[] = "Demo application for HD44780+PCF8574 I2C 1602/1604/2004 LCD display";
    uint8_t i, start = 0;
    
    lcd_init(0x4E, 16, true);
    lcd_fast_puts_xy(4, 0, "LCD TEST");
        
    while(true) {
        lcd_goto_xy(0, 1);
                
        i = start;
        while(STR[i] && i - start < lcd_get_width()) {
            lcd_putchar(STR[i++]);
        }
                  
        if(STR[i]) {
            if(start == 0) {
                _delay_ms(1000);
            } else {
                _delay_ms(500);
            }

            ++start;
        } else {
            _delay_ms(1000);

            start = 0;
        }
    }
}
