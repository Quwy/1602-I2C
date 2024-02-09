/*********************************************
Project : I2C to LCD Interface-Routine
Port PCF8574 :  7  6  5  4  3  2  1  0
               D7 D6 D5 D4 BL EN RW RS
**********************************************/

#ifndef __LCD_H__
#define __LCD_H__

#include <stdint.h>
#include <stdbool.h>

void lcd_init(const uint8_t address, const uint8_t columns, const bool backlight);
void lcd_goto_xy(const uint8_t x, const uint8_t y);
void lcd_clear(void);
void lcd_putchar(const char c);
void lcd_safe_puts(const char *str);
void lcd_fast_puts(const char *const str);
bool lcd_fast_puts_xy(const uint8_t x, const uint8_t y, const char *str);
void lcd_backlight(const bool state, const bool instant);
bool lcd_get_backlight(void);
uint8_t lcd_get_width(void);
uint8_t lcd_get_x(void);
uint8_t lcd_get_y(void);

#endif /* __LCD_H__ */
