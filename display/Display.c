/**
  * Software to interface analog inputs to trolling motor.
  * This uses the lED board to display input position.
  */
  
//#include <stdio.h>

#include <avr/io.h>
//#include <avr/interrupt.h>
#include <util/delay.h>
//#include <avr/wdt.h>
//#include "Display.h"



#define PIN_MSK 0x80
#define PTDIN	0x02

#define SPD_BIT 0x80
#define DIR_OFFSET 3
#define NEG_BIT 0x40
#define VAL_MSK 0x38

#define CMD_BIT 0x04
#define CTRL_MSK 0x03
#define SIP 0
#define PUFF 1
#define LONG_SIP 2
#define LONG_PUFF 3

#define OFF 0
#define ON 1

unsigned char USART_Receive(void);
void clearStates(void);
void lightDisplay(void);
void display(void);

unsigned int dir[8];
unsigned int spd[5];
unsigned int ctrl[4];

int main(void){
   
 
   
   
   /* Set pins as output, high->output, low->input */
   DDRA = 0xFF;          /* Pin for LED out */
   DDRB = 0xFF;          /* Pin for LED out */
   
   DDRC = 0xFF;          /* Pin for LED out */
   DDRD = 0xFF;          /* Pin for LED out */
   unsigned char input;
   unsigned char value;
   unsigned char control;

      PORTA = 0x00;
      PORTB = 0x00;
      PORTC = 0x00;
      PORTD = 0x00;
      



   while (1){
    PORTD ^=(ON<<2);
    _delay_ms(1000);  
    


   }
  /* 
   while(1) {
      clearStates();      
      input = USART_Receive();
      
      control = input & CTRL_MSK;
      if (input & CMD_BIT) {
         ctrl[LONG_SIP]  = control == LONG_SIP                ? ON : OFF;
         ctrl[LONG_PUFF] = control == LONG_PUFF               ? ON : OFF;
         ctrl[SIP]       = control == SIP  || ctrl[LONG_SIP]  ? ON : OFF;
         ctrl[PUFF]      = control == PUFF || ctrl[LONG_PUFF] ? ON : OFF;
      }

      value = input&VAL_MSK >> 3;
      if (input & SPD_BIT) { //SPEED
          if (input & NEG_BIT) { //REVERSE
            spd[0] = ON;
          }
          else { //FORWARD
            switch (value) {
               default:
               case 4:
                  spd[4] = ON;
               case 3:
                  spd[3] = ON;
               case 2:
                  spd[2] = ON;
               case 1:
                  spd[1] = ON;
                  break;
               case 0:
                  break;
            }
         } 
      }
      else { //DIRECTION
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
      }
      
      display();
   }

   return 0;
   */
}

unsigned char USART_Receive(void) {
   while (!(UCSR0B & (1<<RXC0)));

   return UDR0;
}

void clearStates(void) {
   /*int i;
   for (i = 0; i < 8; i++) {
      dir[i] = OFF;
      if (i < 6) {
         spd[i] = OFF;
         if (i < 4) {
            ctrl[i] = OFF;
         }
      }
   }*/
}

void display(void) {
   PORTA = 0;
   PORTB = 0;
   PORTC = 0;
   PORTD = 0;
   
/*   PORTD |= ctrl[SIP] << 2;
   PORTD |= spd[4] << 3;
   PORTD |= spd[3] << 4;
   PORTD |= spd[2] << 5;
   PORTD |= spd[1] << 6;
   PORTD |= ON < 7;

   PORTC |= ctrl[LONG_PUFF] << 0;
   PORTC |= dir[1+DIR_OFFSET] << 1;
   PORTC |= dir[2+DIR_OFFSET] << 2;
   PORTC |= dir[3+DIR_OFFSET] << 3;
   PORTA |= dir[4+DIR_OFFSET] << 0;

   PORTB |= ctrl[LONG_SIP] << 3;
   PORTB |= dir[DIR_OFFSET+1-1] << 2;
   PORTB |= dir[DIR_OFFSET+1-2] << 1;
   PORTB |= dir[DIR_OFFSET+1-3] << 0;
   PORTC |= dir[DIR_OFFSET+1-4] << 7;

   PORTC |= ctrl[PUFF] << 6;
   PORTC |= spd[0] << 5;
*/
}

void lightDisplay(void) {
//   int i;
//   PORTD |= ON << 2;
//   _delay_ms(1000);

//   clearStates();
   display();
//   _delay_ms(1000);

/*   for (i = 0; i < 8; i++) {
      dir[i] = ON;
      if (i < 6) {
         spd[i] = ON;
         if (i < 4) {
            ctrl[i] = ON;
         }
      }
      display();
      _delay_ms(1000);
   }
*/
}
