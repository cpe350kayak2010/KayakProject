/**
  * Software to interface analog inputs to trolling motor.
  * This uses the lED board to display input position.
  */
  
//#include <stdio.h>

#define F_CPU 1000000

#include <avr/io.h>
//#include <avr/interrupt.h>
#include <util/delay.h>
//#include <avr/wdt.h>
//#include "Display.h"



#define PIN_MSK 0x80
#define PTDIN	0x02

#define SPD_BIT 0x80
#define SPD_OFFSET 1
#define DIR_OFFSET 3
#define NEG_BIT 0x40
#define VAL_MSK 0x38

#define CMD_BIT 0x04
#define CTRL_MSK 0x03
#define SIP 1
#define PUFF 0
#define LONG_SIP 2
#define LONG_PUFF 3

#define OFF 0
#define ON 1

unsigned char USART_Receive(void);
void clearStates(void);
void clearDir(void);
void clearSpeed(void);
void clearCtrl(void);
void lightDisplay(void);
void displaySpeed(void);
void displayDirection(void);


unsigned int dir[8];
unsigned int spd[6];
unsigned int ctrl[4];
unsigned int tPortD;

int main(void){

	//setup USART recive
	UCSR0B = (1<<RXEN0);
	UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);
	UBRR0 = 25; // 25 = 2400 baud; 6 = 9600 baud
	 

   /* Set pins as output, high->output, low->input */
   DDRA = 0xFF;          /* Pin for LED out */
   DDRB = 0xFF;          /* Pin for LED out */
   
   DDRC = 0xFF;          /* Pin for LED out */
   DDRD = 0xFE;          /* Pin for LED out */

   unsigned char input;
   unsigned char value;
   unsigned char control;

  	PORTA = 0xFF;
  	PORTB = 0xFF;
  	PORTC = 0xFF;
  	PORTD = 0xFF;
    _delay_ms(1000);

	/*  
  	while (1){
   		clearStates();
		lightDisplay();
   	}*/
   	/*
	int i = 0;
	unsigned int states[4] = {0x27, 0xA0, 0x56, 0xD4};  */
	/*
	while (1) {
		unsigned char in = USART_Receive();

		for (int i = 0; i < 8; i++) {
			dir[i] = (in>>(7-i))&0x1;
		}
		displayDirection();
	}
	*/

   while(1) {
      //clearStates();      
      input = USART_Receive();
      
	  //input = 0x27;
	/*  _delay_ms(2000);
	  input = states[i];
	  i++;
	  i = i%4;*/

      control = input & CTRL_MSK;
      if (input & CMD_BIT) {
         ctrl[LONG_SIP]  = control == LONG_SIP                ? ON : OFF;
         ctrl[LONG_PUFF] = control == LONG_PUFF               ? ON : OFF;
         ctrl[SIP]       = control == SIP  || ctrl[LONG_SIP]  ? ON : OFF;
         ctrl[PUFF]      = control == PUFF || ctrl[LONG_PUFF] ? ON : OFF;
      } else {
	  	 clearCtrl();
	  }

      value = (input&VAL_MSK) >> 3;
      if (input & SPD_BIT) { //SPEED
	  	  clearSpeed();
          if (input & NEG_BIT) { //REVERSE
            spd[0] = ON;
          }
          else { //FORWARD
            switch (value) {
               default:
               case 4:
                  spd[4+SPD_OFFSET] = ON;
               case 3:
                  spd[3+SPD_OFFSET] = ON;
               case 2:
                  spd[2+SPD_OFFSET] = ON;
               case 1:
                  spd[1+SPD_OFFSET] = ON;
                  break;
               case 0:
			   	  spd[0+SPD_OFFSET] = ON;
                  break;
            }
         } 
		 displaySpeed();
      }
      else { //DIRECTION
	  	 clearDir();
         if (input & NEG_BIT) { //LEFT
            switch (value) {
               default:
               case 4:
                  dir[DIR_OFFSET+1-4] = ON;
               case 3:
                  dir[DIR_OFFSET+1-3] = ON;
               case 2:
                  dir[DIR_OFFSET+1-2] = ON;
               case 1:
                  dir[DIR_OFFSET+1-1] = ON;
                  break;
               case 0:
                  break;
            }
         }
         else { //RIGHT
            switch (value) {
               default:
               case 4:
                  dir[4+DIR_OFFSET] = ON;
               case 3:
                  dir[3+DIR_OFFSET] = ON;
               case 2:
                  dir[2+DIR_OFFSET] = ON;
               case 1:
                  dir[1+DIR_OFFSET] = ON;
                  break;
               case 0:
                  break;
            }
         } 
		 displayDirection();
      }
   }

   return 0;
   
}

unsigned char USART_Receive(void) {
   while (!(UCSR0A & (1<<RXC0)));

   return UDR0;
}

void clearStates(void) {
   int i;
   for (i = 0; i < 8; i++) {
      dir[i] = OFF;
      if (i < 6) {
         spd[i] = OFF;
         if (i < 4) {
            ctrl[i] = OFF;
         }
      }
   }
}

void clearDir(void) {
	int i;
	for( i=0; i<8; i++) dir[i] = OFF;
}

void clearSpeed(void) {
	int i;
	for( i=0; i<6; i++) spd[i] = 0;
}

void clearCtrl(void) {
	int i;
	for( i=0; i<4; i++) ctrl[i] = 0;
}

void displaySpeed(void) {
   PORTD = spd[4+SPD_OFFSET] ? PORTD|(ON<<3) : PORTD&~(ON<<3); 
   PORTD = spd[3+SPD_OFFSET] ? PORTD|(ON<<4) : PORTD&~(ON<<4); 
   PORTD = spd[2+SPD_OFFSET] ? PORTD|(ON<<5) : PORTD&~(ON<<5); 
   PORTD = spd[1+SPD_OFFSET] ? PORTD|(ON<<6) : PORTD&~(ON<<6); 
   PORTD = spd[0+SPD_OFFSET] ? PORTD|(ON<<7) : PORTD&~(ON<<7); 
   PORTC = spd[0]            ? PORTC|(ON<<6) : PORTC&~(ON<<6); 

   PORTD = ctrl[SIP]         ? PORTD|(ON<<2) : PORTD&~(ON<<2);
   PORTC = ctrl[PUFF]        ? PORTC|(ON<<5) : PORTC&~(ON<<5); 
   PORTC = ctrl[LONG_PUFF]   ? PORTC|(ON<<0) : PORTC&~(ON<<0);
   PORTB = ctrl[LONG_SIP]    ? PORTB|(ON<<3) : PORTB&~(ON<<3);  
}

void displayDirection(void) {
 
   PORTC = dir[1+DIR_OFFSET] ? PORTC|(ON<<1) : PORTC&~(ON<<1);
   PORTC = dir[2+DIR_OFFSET] ? PORTC|(ON<<2) : PORTC&~(ON<<2);
   PORTC = dir[3+DIR_OFFSET] ? PORTC|(ON<<3) : PORTC&~(ON<<3);
   PORTA = dir[4+DIR_OFFSET] ? PORTA|(ON<<0) : PORTA&~(ON<<0);

   PORTB = dir[DIR_OFFSET+1-1] ? PORTB|(ON<<2) : PORTB&~(ON<<2); 
   PORTB = dir[DIR_OFFSET+1-2] ? PORTB|(ON<<1) : PORTB&~(ON<<1); 
   PORTB = dir[DIR_OFFSET+1-3] ? PORTB|(ON<<0) : PORTB&~(ON<<0); 
   PORTC = dir[DIR_OFFSET+1-4] ? PORTC|(ON<<7) : PORTC&~(ON<<7); 

   PORTD = ctrl[SIP]         ? PORTD|(ON<<2) : PORTD&~(ON<<2);
   PORTC = ctrl[PUFF]        ? PORTC|(ON<<5) : PORTC&~(ON<<5); 
   PORTB = ctrl[LONG_SIP]    ? PORTB|(ON<<3) : PORTB&~(ON<<3); 
   PORTC = ctrl[LONG_PUFF]   ? PORTC|(ON<<0) : PORTC&~(ON<<0);
}

void lightDisplay(void) {
  	int i;

   clearStates();
   displaySpeed();
   displayDirection();
   _delay_ms(1000);

   for (i = 0; i < 8; i++) {
      dir[i] = ON;
      if (i < 6) {
         spd[i] = ON;
         if (i < 4) {
            ctrl[i] = ON;
         }
      }
      displaySpeed();
      displayDirection();
      _delay_ms(1000);
   }

}
