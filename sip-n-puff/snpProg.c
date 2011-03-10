/* Sip-n-Puff interpreter board code. This board interprets user input from the
   sip-n-puff board into motor commands, and sends them out some sort of 
   serial interface (bit-banged SPI I think). -AJH */

/* Sip-n-puff User interface protocol

   short sip: stop, then reverse
   long/continuous sip: turn right. stops turning when sip stops
   
   short puff: increase speed (less reverse/more forward)
   long/continuous puff: turn left. stops turning when sip stops
   */

/* 
   Outputs
   Long sip: PB0
   Long puff: PB1
   */
#include "snpProg.h"

unsigned int iPrevSip,iPrevPuff,iTurn,iSpd,iOutTurn,iOutSpeed,iIdle;

/* interpret enumerated types into data to send to our D/A convertor to
   produce the proper output voltage */
void setOutData()
{
   // TODO: rewrite this for a single backward speed
	iOutSpeed=(0x7<<12) | ((iSpd*496)+64);
	iOutTurn=(0xF<<12) | ((iTurn*1984)+64);
}

/* interpret data from the sip sensor. returns 0 if no chamges were made,
   1 if speed was changed, and 2 if direction was changed. -AJH */
int checkSip()
{
	if((PINA&0x1)==0)		// Check to see if pin is logic high
	{
      PORTB &= ~(1 << PB0); // disable long sip output
		if(iTurn==RIGHT)
		{
			iIdle++; /* if the user has stopped sipping, cancel the turn. -AJH */
			iPrevSip=0;
			if(iIdle==5)
			{
				iTurn=CENTER;
				iIdle=0;
				return 2;
			}
			return 0;
		}
		if(iPrevSip==0)		// Check Counter
		{		
			return 0;		// Still nothing, Return no change
		}
		if(iSpd<=ZERO && iPrevSip!=0)	// If we are going reverse, speed up reverse
		{
			if(iSpd!=REV100)
			{	
				//iSpd--;
            // one speed in reverse: FULL POWER!
            //  we'll limit the reverse speed on the main controller
            iSpd = REV100;
			}
			iPrevSip=0;
			return 1;		// Update Speed
		} else { 			// if we are not going reverse
			iSpd=ZERO;		// Stop the motor
			iPrevSip=0;
			return 1;		// Update Speed
		}
		iPrevSip=0;			// Reset counter
	} else {
		iPrevSip++;			// Update Counter
		if(iPrevSip==10)	// Becomes a long Sip
		{
         PORTB |= (1 << PB0); // enable long sip output
			iTurn=RIGHT;	// Update Direction
			iPrevSip=0;		// Reset counter 
			return 2;		// Update Turn
		}
	}
	return 0;
}

/* interpret data from the puff sensor. returns 0 if no chamges were made,
   1 if speed was changed, and 2 if direction was changed. -AJH */
int checkPuff()
{
	if((PINA&0x2)==0) 		// check Puff Pin
	{
      PORTB &= ~(1 << PB1); // disable long puff output
		if(iTurn==LEFT)
		{
			iIdle++; /* if the user has stopped puffing, cancel the turn. -AJH */
			iPrevPuff=0;
			if(iIdle==5)
			{
				iTurn=CENTER;
				iIdle=0;
				return 2;
			}
			return 0;
		}
		if(iPrevPuff==0)	// if the pin is not high, return
		{
			return 0;		// dont update anything
		}
		if(iSpd!=FWD100 && iPrevPuff!=0)
		{
         if(iSpd == REV100) iSpd = ZERO;
         else iSpd++;  // Speed up if we have been puffing for a while and stop
			iPrevPuff=0;
			return 1;		// update speed
		}
		iPrevPuff=0;
	} else {
		iPrevPuff++;		//Iterate iPrevPuff
		if(iPrevPuff==10)	//Becomes a long Puff
		{
         PORTB |= (1 << PB1); // enable long puff output
			iTurn=LEFT;		// update Turn status
			iPrevPuff=0;	// reset Puff Counter
			return 2;		// update Turn
		}
	}
	return 0;
}			


// Interrupt for Timer Counter
/* this gets run on a very regular interval, checks the state of the sip-n-puff
   pins, and optionally outputs new control data over SPI. -AJH */
ISR(TIM0_OVF_vect,ISR_BLOCK)
{
	int iTemp,iTemp2;

   /* check the sip-n-puff pins */
	iTemp=checkSip();
	iTemp2=checkPuff();
	setOutData();				// Set Turn and Spd based on x/4095
	if(iTemp==1 || iTemp2==1)	// update speed
	{
      spiWrite(iOutSpeed);
		_delay_loop_1(100);
	}
	if(iTemp==2 || iTemp2==2)	// update turn
	{
      spiWrite(iOutTurn);
	}	
}

/* initialize our pins to be inputs/outputs as appropriate for SPI. -AJH */
int spiInit()
{
	USICR=USICR|0x10;
	USISR=USISR|0x00;
	DDRA=DDRA|0x38;
	PORTA=PORTA|0x38;
	return 0;
}	

/* 
   Write our two bytes of steering control data out over the SPI bus. The first
   byte is the MSB of the steering, and has its highest bit SET.  -AJH */
/*
   write our two bytes of speed control data out over the SPI bus. The first
   byte is the MSB of the speed, and has its highest bit CLEARED. -AJH */

/* write the two bytes of an int to the SPI port */
void spiWrite(unsigned int out)
{
	int i;
	PORTA=PORTA & 0xF7;
	_delay_loop_1(5);
	USICR=0x11;
	_delay_loop_1(10);
   /* set up our first output byte. -AJH */
	//USIDR=(0x7<<4)+(out>>8);
	USIDR=(out>>8);
   /* Transmit our first byte! -AJH */
	for(i=0;i<8;i++)
	{
		USICR=0x11;
		_delay_loop_1(10);
		USIDR=USIDR<<1;
		USIDR=USIDR>>1;
		USICR=0x11;
		USIDR=USIDR<<1;
		_delay_loop_1(10);
	}
   /* set up our second output byte. -AJH */
	USIDR=out&0xFF;
   /* Transmit our second byte! -AJH */
	for(i=0;i<8;i++)
	{
		USICR=0x11;
		_delay_loop_1(10);
		USIDR=USIDR<<1;
		USIDR=USIDR>>1;
		USICR=0x11;
		USIDR=USIDR<<1;
		_delay_loop_1(10);
	}
	USICR=0x11;
	_delay_loop_1(20);
	PORTA=PORTA|0x8;
}

/* 13.15, 20.49, 27.99,35.46 */
/* random numbers... interesting. They probably mean something to someone.
   This is the entry point into our program; it should initilize the interrupts
   and set up the I/O, then go into an inifinite loop while our interrupt
   routine polls the input at regular intervals and produces output. -AJH */
int main()
{//l,f,ri,re
   // set long sip and long puff pins as outputs
   DDRB |= (1 << PB0) | (1 << PB1);
	// Initialize Timer
	TCCR0A=0x0;
	TCCR0B=0x4;
	TIMSK0=0x1;
	spiInit();					// Initialize SPI
	// Initialize Sip and Puff counters
	iPrevPuff=0;
	iPrevSip=0;
	iIdle=0;

   /* Zero our state data and prepare for operation. -AJH */
	iTurn=CENTER;
	iSpd=ZERO;
	setOutData();
   spiWrite(iOutSpeed);
	_delay_loop_2(200);
   spiWrite(iOutTurn);
	// Enable interrupts
	sei();
   while(1); /* main loop does nothing; everything handled by interrupts */
	return 0;
}
