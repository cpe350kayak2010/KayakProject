/* Sip-n-Puff interpreter board code. This board interprets user input from the
   sip-n-puff board into motor commands, and sends them out some sort of 
   serial interface (bit-banged SPI I think). -AJH */

/* Sip-n-puff User interface protocol
   This is extracted as best I can from the code

   short sip: stop, then reverse
   long/continuous sip: turn right. stops turning when sip stops?
   
   short puff: increase speed (less reverse/more forward)
   long/continuous puff: turn left. stops turning when sip stops?
   */
#include "snpProg.h"

unsigned int iPrevSip,iPrevPuff,iTurn,iSpd,iOutTurn,iOutSpeed,iIdle;

/* Some sort of interpretation of the internal state into an outputtable form,
   I'm guessing.  -AJH */
/* correction: I think this maps the internal enums for speed and direction
   onto a linear speed/direction scale */
int setOutData()
{
	iOutSpeed=(iSpd*496)+64;
	iOutTurn=(iTurn*1984)+64;
	return 0;
}

/* interpret data from the sip sensor. returns 0 if no chamges were made,
   1 if speed was changed, and 2 if direction was changed. -AJH */
int checkSip()
{
	if((PINA&0x1)==0)		// Check to see if pin is logic high
	{
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
				iSpd--;
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
			iSpd++;			// Speed up if we have been puffing for a while and stop
			iPrevPuff=0;
			return 1;		// update speed
		}
		iPrevPuff=0;
	} else {
		iPrevPuff++;		//Iterate iPrevPuff
		if(iPrevPuff==10)	//Becomes a long Puff
		{
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
		spiWriteSpeed();
		_delay_loop_1(100);
	}
	if(iTemp==2 || iTemp2==2)	// update turn
	{
		spiWriteTurn();
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
int spiWriteTurn()
{
	int i;
	PORTA=PORTA & 0xF7;
	_delay_loop_1(5);
	USICR=0x11;
	_delay_loop_1(10);
   /* set up our first output byte. -AJH */
	USIDR=(0xF<<4)+(iOutTurn>>8);
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
	USIDR=iOutTurn&0xFF;
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
	return 0;
}

/*
   write our two bytes of speed control data out over the SPI bus. The first
   byte is the MSB of the speed, and has its highest bit CLEARED. -AJH */
int spiWriteSpeed()
{
	int i;
	PORTA=PORTA & 0xF7;
	_delay_loop_1(5);
	USICR=0x11;
	_delay_loop_1(10);
   /* set up our first output byte. -AJH */
	USIDR=(0x7<<4)+(iOutSpeed>>8);
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
	USIDR=iOutSpeed&0xFF;
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
	return 0;
}

/* 13.15, 20.49, 27.99,35.46 */
/* random numbers... interesting. They probably mean something to someone.
   This is the entry point into our program; it should initilize the interrupts
   and set up the I/O, then go into an inifinite loop while our interrupt
   routine polls the input at regular intervals and produces output. -AJH */
int main()
{//l,f,ri,re
	volatile int i,j,jTop;
	int aiStartupSpeed[4]={FWD100,FWD100,REV100,REV100};
	int aiStartupTurn[4]= {LEFT,LEFT,RIGHT,RIGHT};
	// Initialize Timer
	TCCR0A=0x0;
	TCCR0B=0x4;
	TIMSK0=0x1;
	spiInit();					// Initialize SPI
	// Initialize Sip and Puff counters
	iPrevPuff=0;
	iPrevSip=0;
	iIdle=0;
	jTop=45;
	spiWriteSpeed();
	spiWriteTurn();
	for(j=0;j<jTop;j++)
	{
		_delay_loop_2(40000);
	}
	jTop=40;
   /* This looks like a test loop that sends a bunch of data so that a user can
      confirm that data is being properly transmitted from the spi-n-puff 
      control board (this code) to the primary processor.
      It should cycle Forward and Left for "a while", then Backwards and Right
      for "a while"
      
      this is probably trying to do joystick calibration

      Honestly, this looks kinda dangerous if 1) this controller accidentally
      resets or 2) the user just isn't expecting it. -AJH */
	for(i=0;i<4;i++)
	{
		for(j=0;j<jTop;j++)
		{
			iTurn=aiStartupTurn[i];
			iSpd=aiStartupSpeed[i];
			setOutData();
			spiWriteSpeed();
			_delay_loop_2(200);
			spiWriteTurn();
			_delay_loop_2(40000);
		}
	}
   /* Zero our state data and prepare for operation. -AJH */
	iTurn=CENTER;
	iSpd=ZERO;
	setOutData();
	spiWriteSpeed();
	_delay_loop_2(200);
	spiWriteTurn();
	// Start ISR
	sei();
	return 0; /* this probably ought to be an infinite loop. -AJH */
}
