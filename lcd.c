/*********************************************
Port PCF8574 :  7  6  5  4  3  2  1  0
               D7 D6 D5 D4 BL EN RW RS
**********************************************/

#include <util/delay.h>
#include <string.h>

#include "lcd.h"
#include "twi.h"

#define BL 3
#define EN 2
#define RW 1
#define RS 0

#define LCD_MAX_SINGLE_BUF 5

typedef enum {
    LCD_SEND_DATA, 
    LCD_SEND_COMMAND
} lcd_send_data_kind_t;

typedef enum {
    LCD_DATA_8BIT, 
    LCD_DATA_4BIT
} lcd_send_data_size_t;

struct {
    uint8_t address, width;
    uint8_t x, y;
    bool bl;
} lcd_context;

static const uint8_t lcd_base[4] = {0x00, 0x40, 0x10, 0x50};
static __flash const uint8_t lcd_start[] = {0b0011, 0b0011, 0b0011, 0b0010, // startup sequence
                                            0b00101000, // 4 bit data bus, lines=2, font=5x8
                                            0b00001100, // display=on, cursor=off, blink=off
                                            0b00000110, // pos=increment, scroll=false
                                            0b00000001}; // cls

static void lcd_write_callback(const uint8_t idx, const uint8_t size) {
    switch(idx) {
        case 0: // upper half of the byte, backlight, E is set, RS
        case 2: // lower half of the byte,  backlight, E is set, RS
            _delay_us(0.200); // first half of the minimal cycle of the E (high level)
            break;
        case 4: // set E for next cycle
             _delay_us(0.050); // delay between bring up E and data value (optional)
            break;
        case 1: // first strobe (clear E)
        case 3: // second strobe (clear E)
            _delay_us(0.200); // second half of the minimal cycle of the E (clear)
            break;
    }
}

static uint8_t lcd_pack_command(uint8_t buf[], const uint8_t data, const lcd_send_data_size_t data_size, const lcd_send_data_kind_t data_kind) {
    const uint8_t blf = (lcd_context.bl ? (1 << BL) : 0);
    const uint8_t cmf = (data_kind == LCD_SEND_DATA ? (1 << RS) : 0);
    uint8_t idx = 0;
  
    if(data_size == LCD_DATA_8BIT) {
        buf[idx] = (data & 0xf0) | blf | (1 << EN) | cmf; // 0. high half of the byte, backlight, E is set, RS (E line normally set after previous packet, so it just keep state)
        buf[idx + 1] = buf[idx] & ~(1 << EN);             // 1. same as above, but clearing E (first strobe)
        idx += 2;
    }
  
    buf[idx] = (data << 4) | blf | (1 << EN) | cmf; // 2. low half of the byte and control lines sabe as above
    buf[idx + 1] = buf[idx] & ~(1 << EN);           // 3. clearing E (second strobe)
    idx += 2;
  
    buf[idx] = blf | (1 << EN); // 4. clearing data bits, backlignt not cjanges, rising E for the next packet, clearing RS (and goto 0 of the next packet)
    idx += 1;
  
    return idx;
}

static bool lcd_write_single(const uint8_t data, const lcd_send_data_size_t data_size, const lcd_send_data_kind_t data_kind) {
    uint8_t buf_size, buf[LCD_MAX_SINGLE_BUF];
    bool res;
  
    res = twi_write_begin(lcd_context.address);
    if(res) {
        buf_size = lcd_pack_command(buf, data, data_size, data_kind);
        res = twi_write_bytes(buf, buf_size, lcd_write_callback);
    }
    twi_write_end();
  
    return res;
}

static bool lcd_write_data(const uint8_t *data, uint8_t size) {
    bool res;
    uint8_t buf_size, buf[LCD_MAX_SINGLE_BUF];
  
    res = twi_write_begin(lcd_context.address);
    while(res && size-- > 0) {
        buf_size = lcd_pack_command(buf, *(data++), LCD_DATA_8BIT, LCD_SEND_DATA);
        res = twi_write_bytes(buf, buf_size, lcd_write_callback);
    }
    twi_write_end();
  
    return res;
}

/***************** INTERFACE *****************/

void lcd_init(const uint8_t address, const uint8_t columns, const bool backlight) {
    lcd_context.address = address;
    lcd_context.bl = backlight;
    lcd_context.width = columns;
    lcd_context.x = 0;
    lcd_context.y = 0;
    
    twi_init_master();
    _delay_ms(30.0);
    
    for(uint8_t i = 0; i < sizeof(lcd_start); ++i) {
        lcd_write_single(lcd_start[i], (i <= 3 ? LCD_DATA_4BIT : LCD_DATA_8BIT), LCD_SEND_COMMAND);
        if(i == 0) {
            _delay_ms(4.1);
        } else {
            _delay_ms(2.0);
        }
    }
    
    _delay_ms(10.0);
}

void lcd_goto_xy(const uint8_t x, const uint8_t y) {
    lcd_write_single(0x80 | (lcd_base[y] + x), LCD_DATA_8BIT, LCD_SEND_COMMAND);
    lcd_context.x = x;
    lcd_context.y = y;
    _delay_us(100);
}

void lcd_clear(void) {
    lcd_write_single(0x01, LCD_DATA_8BIT, LCD_SEND_COMMAND);
    lcd_context.x = lcd_context.y = 0;
    _delay_ms(1.18);
}

void lcd_putchar(const char c) {
    lcd_write_single(c, LCD_DATA_8BIT, LCD_SEND_DATA);
    if(++lcd_context.x >= lcd_context.width) {
        lcd_goto_xy(0, lcd_context.y + 1);
    }
}

void lcd_safe_puts(const char *str) {
    while(*str != 0) {
        lcd_putchar(*(str++));
    }
}

void lcd_fast_puts(const char *const str) {
    const uint8_t len = strlen(str);
        
    lcd_write_data((uint8_t *) str, len);
    lcd_context.x += len;
}

bool lcd_fast_puts_xy(const uint8_t x, const uint8_t y, const char *str) {
    bool res;
    uint8_t buf_size, buf[LCD_MAX_SINGLE_BUF];
  
    res = twi_write_begin(lcd_context.address);
    if(res) {
        lcd_context.x = x + strlen(str);
        lcd_context.y = y;
    
        buf_size = lcd_pack_command(buf, 0x80 | (lcd_base[y] + x), LCD_DATA_8BIT, LCD_SEND_COMMAND);
        res = twi_write_bytes(buf, buf_size, lcd_write_callback);
        _delay_us(100);
    
        while(res && *str != 0) {
            buf_size = lcd_pack_command(buf, *((uint8_t*) str++), LCD_DATA_8BIT, LCD_SEND_DATA);
            res = twi_write_bytes(buf, buf_size, lcd_write_callback);
        }
    }
    twi_write_end();
  
    return res;
}

void lcd_backlight(const bool state, const bool instant) {
    lcd_context.bl = state;
  
    if(instant) {
        if(twi_write_begin(lcd_context.address)) {
            twi_write_byte((lcd_context.bl ? (1 << BL) : 0) | (1 << EN));
        }    
        twi_write_end();
    }
}

bool lcd_get_backlight(void) {
    return lcd_context.bl;
}

uint8_t lcd_get_width(void) {
    return lcd_context.width;
}

uint8_t lcd_get_x(void) {
    return lcd_context.x;
}

uint8_t lcd_get_y(void) {
    return lcd_context.y;
}
