/* Sip-N-Puff header file
   This contains the function prototypes and enums for the sip-n-puff 
   controller code. Not really much to see; only included from the sip-n-puff
   controller itself. -AJH */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay_basic.h>

/* enums for speed and direction */
enum {RIGHT,CENTER,LEFT};
enum {REV100,REV75,REV50,REV25,ZERO,FWD25,FWD50,FWD75,FWD100};

int setOutData();
int checkSip();
int checkPuff();
int spiInit();
int spiWriteSpeed();
int spiWriteTurn();
