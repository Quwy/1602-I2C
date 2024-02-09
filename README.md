# 1602-I2C
Standalone USI driver for 1602 LCD display with I2C bus (HD44780 + PCF8574).

![LCD photo](https://github.com/Quwy/1602-I2C/blob/main/images/LCD_photo.jpg?raw=true)

Driver for popular LCD display with 16x2, 16x4, 20x4 resolution and I2C interface provided by additional PCF8574 shift register.
Applicable for any AVR ATtiny chip with USI interface support: 24/44/84, 25/45/85, 261/461/861, 87/167, 2313/4313, 1634.

* 100% pure C
* Do not require any external ~~Arduino~~ libraries
* Minimal application about 750 bytes

![Demo circuit live photo](https://github.com/Quwy/1602-I2C/blob/main/images/LCD_demo.jpg?raw=true)

Included sample test application for ATtiny25/45/85 and simulated circuit for Proteus.

![Proteus simulation screenshot](https://github.com/Quwy/1602-I2C/blob/main/images/LCD_proteus.png?raw=true)
