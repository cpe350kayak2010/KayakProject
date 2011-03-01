/**
  * Software to interface analog inputs to trolling motor.
  * This uses the lED board to display input position.
  */
  
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "Display.h"


#define PIN_MSK 0x80
#define PTDIN	0x02

#define SPD_BIT 0x80
#define SPD_OFFSET 1
#define DIR_OFFSET 3
#define NEG_BIT 0x40
#define VAL_MSK 0x38

#define CMD_BIT 0x04
#define SIP 0
#define PUFF 1
#define LONG_SIP 2
#define LONG_PUFF 3

#define OFF 0
#define ON 1

unsigned char USART_Receive(void);

unsigned int dir[8];
unsigned int spd[6];
unsigned int ctrl[4];

int main(void){
   /* Set pins as output, high->output, low->input */
   DDRB = 0;          /* Pin for serial in */
   DDRC = 1;          /* Pin for LED out */
   unsigned char input;
   unsigned char value;

   while(1) {
      clearStates();      
      input = USART_Receive;
      
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
            switch (value) {
               default:
               case 2:
                  spd[2-1] = ON;
               case 1:
                  spd[1-1] = ON;
                  break;
               case 0:
                  break;
            }
         
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
                  break;
            }
         } 
      }
      else { //DIRECTION
         if (input & NEG_BIT) { //LEFT
            switch (value) {
               default:
               case 4:
                  dir[4-1] = ON;
               case 3:
                  dir[3-1] = ON;
               case 2:
                  dir[2-1] = ON;
               case 1:
                  dir[1-1] = ON;
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
   }

   return 0;
}

unsigned char USART_Receive(void) {
   while (!(UCSR0B & (1<<RXC0)));

   return UDR0;
}

void clearStates() {
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
