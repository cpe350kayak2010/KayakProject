/**
 * This controls the LCD with the Samsung LCD driver, S6A0069.  The instruction
 * set can be found in the s6A0069 datasheet. This program uses the 8-bit
 * interface: 8 data signals and 3 control signals.
 *
 * @author Jed Rivero
 * @version 1.0  Last Modified: 2/27/2008
 */
 
#include <avr/io.h>
#include <util/delay.h>
#include "LCD.h"

#define RS 5                /* Register select */
#define RW 6                /* Read/Write */
#define EN 7                /* Enable */

/*
 * This function initializes the LCD.
 */
void LCD_init(){

    /* Function Set: 2 Line, 8-bit, 5x7 dots */
    PORTD = 0x38;
    PORTA &= ~((1<<RS) | (1<<RW));
    PORTA |= (1<<EN);
	PORTA &= ~(1<<EN);
    _delay_us(50);             /* Delay for 200 microseconds */
	
	/* Display on, Cursor off */
    PORTD = 0x0c;
	PORTA &= ~((1<<RS) | (1<<RW));
    PORTA |= (1<<EN);
	PORTA &= ~(1<<EN);
    _delay_us(50);
	
	/* Clear Display */
    PORTD = 0x01;
	PORTA &= ~((1<<RS) | (1<<RW));
    PORTA |= (1<<EN);
	PORTA &= ~(1<<EN);
    _delay_ms(3);                /* Delay for 5 milliseconds */
	
	/* Entry Mode: auto-increment cursor, no shift */
    PORTD = 0x06;
	PORTA &= ~((1<<RS) | (1<<RW));
    PORTA |= (1<<EN);
	PORTA &= ~(1<<EN);
	_delay_us(50);
	
}

/*
 * This function sends commands to the LCD. The parameter com is the command
 * to send to the LCD.
 */
void LCD_command(unsigned char com){

    PORTD = com;
    PORTA &= ~((1<<RS) | (1<<RW));
    PORTA |= (1<<EN);
    PORTA &= ~(1<<EN);
    _delay_ms(3);
}

/*
 * This function writes data to the LCD. The parameter data is the data to write
 * to the LCD.
 */
void LCD_putchar(unsigned char data){
    PORTD = data;
    PORTA |= (1<<RS);
    PORTA &= ~(1<<RW);
    PORTA |= (1<<EN);
    PORTA &= ~(1<<EN);
    _delay_us(100);
}

/*
 * This function writes the string of data to the LCD. The parameter str is
 * the string of data to write to the LCD.
 */
void LCD_write(char *str){

     int i = 0;
     while(str[i])
       LCD_putchar(str[i++]);
}
