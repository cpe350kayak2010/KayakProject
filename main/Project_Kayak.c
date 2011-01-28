/**
  * Software to interface analog inputs to trolling motor.
  * This uses the lED board to display input position.
  */
  
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "Project_Kayak.h"
//#include "LCD.h"

#define NUM_CHANNELS 2    /* Number of channels used in the ADC */
#define INPUT_X 0         /* Channel for x direction of input */
#define INPUT_Y 1         /* Channel for y direction of input */

#define NUM_MODES 3       /* Number of modes for the LCD display */
#define TRUE 1
#define FALSE 0
#define TURN_FACTOR 1     /* Amount to turn per iteration of the main loop */
                          /* a factor of 2 means motor turns 1/2 of */
						  /* distance to destination per loop iteration */
#define ADC_OFFSET 21     /* ADC has an inexplicable offset error of 21 */

#define CENTER 0
#define LEFT 1
#define RIGHT 2

#define PIN_MSK (0x80)
#define PTDIN	(0x02)

unsigned volatile char speed;
unsigned volatile char ucCalMode;
unsigned volatile char x_min, y_min;         /* Minimum x and y positions */
unsigned volatile char x_max, y_max;         /* Maximum x and y positions */
unsigned volatile char x_motor;              /* x and y positions of motor */
unsigned volatile char channel;               /* Current channel of ADC */
                                              /* position */
unsigned volatile char calibrating;           /* Motor is calibrating */
uint16_t volatile motor_range;                /* Time to fully swing motor */
unsigned volatile char motor_pos;

int main(void){


   /* Set pins as output, high->output, low->input */
   DDRB = 0x0f;          /* Pins for motor control */

   /* Port D information
    * Sets pin 7 of port D to be an interrupt
    * Also, sets the pins appropriately to input/output
    */
  // PORTD = PTDIN;
   DDRD = 0x0;
   PORTD = (1 << 5);
   /*End of Port D information*/

   DDRA |= (1<<4);
  // PORTD = 0xfc;

   /* Initialization */
   channel = INPUT_X;
   ADMUX |= channel;     /* ADMUX selects the channel for A2D conversions */
   x_max = y_max = y_min = x_min = 128;

   sei();                /* Enable global interrupts */

   /* Check if input device is connected */
   motor_init();

   ADC_init();

//  while(1)
//  {
// 
//   /* ramp motor output from 0 to 100 to 0, increments of 10, incrementing or
//      decrementing every 5 seconds. Probably a test of the PWM and motor
//      controller circuitry. Commenting for now. -AJH */   
//      uint8_t i = 0;
//      while(i < 2)
//      {
//         _delay_ms(1000);
//         i++;
//      }
//      if(100 < y)
//      {
//         set_reverse= 1;
//      }
//      if(0 > y)
//      {
//         set_reverse = 0;
//      }
//      // Output to motor 
//      output_motor();
//      if(set_reverse)
//         y -=10;
//      else
//         y +=10;
//
//      //Forward/Reverse
//      speed = 10;
//      output_motor();
//      _delay_ms(500);
//
//      speed = 50;
//      output_motor();
//      _delay_ms(500);
//
//      speed = 90;
//      output_motor();
//      _delay_ms(500);
//
//      //Left/Right
//      motor_right(0x0);
//      _delay_ms(500);
//      motor_left(0x0);
//      _delay_ms(500);
//
//      //Sip-N-Puff
//
//  }


   /* Calibrate input device and motor */
   input_calibration();
   motor_calibration();

   //	x_min = y_min = 125;
   //x_max = y_max = 175;

   /* Adjust for offfset in the ADC, ADC is off by 42 */
   if(x_min == 0)
      x_min = 2*ADC_OFFSET;
   if(y_min == 0)
      y_min = 2*ADC_OFFSET;

   while (1);
   
   /* Once connected, reset system using the watchdog timer */
   wdt_enable(WDTO_15MS);
   return 0;
}


/*
 * This function initializes the ADC. 
 */
void ADC_init(void){

   /* Set ADC prescaler to 8 (125KHz sample rate @ 1MHz) */
   ADCSRA |= ((1<<ADPS0) | (1<<ADPS1));

   /* Set ADC reference voltage to AVCC */
   ADMUX |= (1<<REFS0);

   /* Left adjust ADC result for 8 bit precision */
   ADMUX |= (1<<ADLAR); 

   /* Set ADC to Free-Running Mode */
   ADCSRA |= (1<<ADATE); 

   /* Enable ADC */ 
   ADCSRA |= (1<<ADEN);

   /* Enable ADC Interrupt */
   ADCSRA |= (1<<ADIE);

   /* Start ADC Conversions */
   ADCSRA |= (1<<ADSC);
}


/*
 * This function calibrates the input device. Calibration requires the user
 * to move the joystick all the way left, up, right, and down at appropriate
 * times.
 */ 
/* this looks like it requires that the user know about the calibration
   routine and move the joystick accordingly. It appears as though some sort
   of feedback device may be attached to PORTA, pins 5 6 and 7, to provide 
   sequencing information during calibration. -AJH */
void input_calibration(void){
   _delay_ms(200);
   ucCalMode=0;
   /* Move joystick to the left */
   PORTA &= ~((1<<5) | (1<<6) | (1<<7));
   PORTA |= (1<<5);
   ucCalMode++;
   _delay_ms(400);
   if(x_max < 250)
      x_max = x_max+5;       /* x_input and y_input are updated in ISR */
   /* Move joystick forward */
   PORTA &= ~((1<<5) | (1<<6) | (1<<7));
   ucCalMode++;
   _delay_ms(400);
   if(y_max <250)
      y_max = y_max+5;       /* x_input and y_input are updated in ISR */
   /* Move joystick right */
   PORTA &= ~((1<<5) | (1<<6) | (1<<7));
   ucCalMode++;
   _delay_ms(400);
   if(x_min >5)
      x_min = x_min-5;       /* x_input and y_input are updated in ISR */
   /* Move joystick back */
   PORTA &= ~((1<<5) | (1<<6) | (1<<7));
   ucCalMode++;
   _delay_ms(400);
   if(y_min > 5)
      y_min = y_min-5;       /* x_input and y_input are updated in ISR */
   ucCalMode=0;
}

/*
 * This function initializes the motor.
 */
void motor_init(void){

   /* Set fast PWM non-inverting mode to control velocity */
   //TCCR0A |= (1<<COM0A1) | (1<<WGM00) | (1<<WGM01);
   // set phase-correct PWM mode
   TCCR0A = (1<<COM0A1) | (1<<WGM00);

   /* Set up interrupt for feedback sensors */
   //	PCICR |= (1<<PCIE2);          /* Enable interrupt for PCINT23:16 */
   //	PCMSK2 = 0x43;                /* Select which pins are interrupt enabled */

   /* Start PWM with 0 duty cycle */
   OCR0A = 0;                     /* Set duty cycle of PWM to 0 (no speed)*/
   TCCR0B = (1<<CS00);          /* Set prescaler to 1 and start PWM */ 

   /* Enable compare match interrupt to control turning of motor */
   //	TIMSK1 |= (1<<OCIE1A);

   /* Enable timer overflow interrupt */
   //	TIMSK1 |= (1<<TOIE1);     
}

/*
 * This function calibrates the motor. The motor is first turned all the way
 * left. The motor is then turned all the way to the right while a timer times
 * the duration of the swing. Finally, the motor is centered.
 */
/* I'm reasonalby certain that this code is for a previous version of the rear
   motor position sensor, and probably won't work with the current 
   configuration of the kayak. -AJH */
void motor_calibration(void){

   PORTA |= (1<<5) | (1<<6) | (1<<7);
   //PORTD = 0xff;

   /* Move motor to left-most position, ISR stops the motor */
   PORTB &= ~(1<<1);
   PORTB |= (1<<0);
   // while(PINC & 0x01)
   PORTB &= ~(1<<0);

   /* Move motor to right-most position, and time the duration */
   /* ISR records the time */
   calibrating = TRUE;
   PORTB |= (1<<1);
   PORTB |= (1<<0);
   TCCR1B |= (1<<CS12);        /* Set prescaler to 256 and start timer */
   // while((PINC>>1) & 0x01)
   PORTB &= ~(1<<0);
   TCCR1B &= ~(1<<CS12);
   motor_range = TCNT1;

   /* Center the motor */
   PORTB &= ~(1<<1);
   PORTB |= (1<<0);
   // while((PINC>>6) & 0x01)
   PORTB &= ~(1<<0);
   x_motor = 50;
   motor_pos = CENTER;

   /* Set clear on compare match */
   TCCR1B |= (1<<WGM12);

   /* Enable compare match interrupt to control turning of motor */
   TIMSK1 |= (1<<OCIE1A);

   PORTA &= ~((1<<5) | (1<<6) | (1<<7));
   //PORTD = 0;
}

/*
 * This function outputs to the motor.
 */
/* values greater than 50 are forwards, less than 50 are reverse. -AJH */
void output_motor(void){
   if(speed>55 && speed < 100) {
      motor_forward(2*(speed-50));
   } 
   else if(speed<45 && speed > 0) {
      motor_reverse(2*(50-speed));
   } 
   else {
      OCR0A = 0;                     /* Set duty cycle of PWM to 0 */
   }
}

/*
 * This function alters the velocity of the motor. The parameter vel is the
 * velocity in the range from 0 to 100.
 */
void motor_forward(unsigned char vel){
   if((PORTB>>2) & 0x01) {
      OCR0A = 0;                     /* Set duty cycle of PWM to 0 */
      _delay_ms(50);
      PORTB &= ~(1<<2);         /* Clear reverse */
      _delay_ms(50);
   }

   OCR0A = (255*vel)/100;        /* Set duty cycle of PWM */
}

/*
 * This function alters the velocity of the motor and sets the motor to reverse.
 * The parameter vel is the velocity in the range from 0 to 100.
 */
void motor_reverse(unsigned char vel){
   if(!((PORTB>>2) & 0x01)){
      OCR0A = 0;                     /* Set duty cycle of PWM to 0 */
      _delay_ms(50);
      PORTB |= (1<<2);          /* Set reverse */
      _delay_ms(50);
   }
   OCR0A = (255*vel)/100;        /* Set duty cycle of PWM */
}

/*
 * This function moves the motor to the right. An interrupt enabled timer is 
 * used to control duration of turn. The ISR turns off the motor. This function
 * may also increase velocity to ensure that the kayak moves right when the user
 * inputs right. The parameter dist is the duration of the turn with a range 
 * from 0 to 100.
 */
/* this is probably for an older version of the kayak steering sensor, and
   probably won't work. -AJH */
void motor_right(unsigned char dist){

   PORTB |= (1<<0); //set direction
   PORTB |= (1<<1); //turn on
   _delay_ms(1000);
   PORTB &= ~(1<<1); //turn off
}

/*
 * This function moves the motor to the left. An interrupt enabled timer is 
 * used to control duration of turn. The ISR turns off the motor. This function
 * may also increase velocity to ensure that the kayak moves left when the user
 * inputs left. The parameter dist is the duration of the turn with a range 
 * from 0 to 100.
 */
/* this is probably for an older version of the kayak steering sensor, and
   probably won't work. -AJH */
void motor_left(unsigned char dist){

   PORTB &= ~(1<<0); //sets direction
   PORTB |= (1<<1); //turn on
   _delay_ms(1000);
   PORTB &= ~(1<<1); //turn off

}

/*
 * This function is called before main is executed. This function disables the
 * watchdog timer. This is to keep the system from resetting when the watchdog
 * timer expires.  When an input device in just inserted into the system, a
 * watchdog timer is enabled with a timeout of 15 ms.  When the system resets,
 * the timer is still enabled with timeout of 15 ms, and if on, the system will
 * keep on resetting.
 */ 
void wdt_init(void){
   MCUSR = 0;
   wdt_disable();

   return;
}

/*
 * Interrupt routine for the ADC. This reads the analog signals for the input
 * device and the voltage feedback.
 */
/* this routine also contains the logic for kayak control, and for the joystick
   calibration function. It gets called every time the A2D finishes a 
   conversion and has a new sample to be processed by the user. -AJH */
ISR(ADC_vect){
   unsigned volatile char ucTemp;
   int y_input;
   
   ADCSRA &= ~(1<<ADEN);       /* Disable ADC */

   /* read the current A2D reading */
   ucTemp= ADCH;

   /* Get the analog input from the current channel */
   switch(channel){
      case INPUT_X:
        break;
      case INPUT_Y:

         /* calibartion mode 2: Y maximum. -AJH */
         if(ucCalMode==2 && ucTemp > y_max)
         {
            y_max = ucTemp;
         }
         /* calibration mode 4: Y minimum. -AJH */
         if(ucCalMode==4 && ucTemp < y_min)
         {
            y_min = ucTemp;
         }
         y_input=ucTemp;
         /* if we aren't in calibration mode, actually control the kayak.
            -AJH */
         if(ucCalMode==0)
         {
            speed = (y_input-y_min)*100/(y_max-y_min);

            output_motor();
         }
         break;
      default:
         break;
   }

   /* Select next channel */
   channel++;
   if((channel%NUM_CHANNELS) == 0)
      channel = 0;

   ADMUX &= 0xe0;
   ADMUX |= channel;           /* Set the next channel for conversion */
   ADCSRA |= (1<<ADEN);       /* Enable ADC */
   ADCSRA |= (1<<ADSC);       /* Start conversions */
}

/*
 * Interrupt routine for timer 1 compare match.
 */
/* Turn off bit 0 of PORTB every time timer 1 reaches a particular value.
   I have no idea WHY it does this. This might be part of the
   calibration routine? -AJH */
ISR(TIMER1_COMPA_vect){

   PORTB &= ~(1<<0);
}

/*
 * Interrupt for timer 1 overflow. If this happens, calibration failed so
 * disable system from further use.
 */
ISR(TIMER1_OVF_vect){

   PORTB |= (1<<0);
}
