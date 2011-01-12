#include <avr/io.h>

// initialize the ADC
void adc_init() {
   // reference AVCC (5V), adjust right, no mux selection. page 248
   ADMUX = (1 << REFS0);

   // ADC enable, no auto trigger, no interrupt, prescaler: /8. page 250
   ADCSRA = ((1 << ADEN) | (1<<ADPS1) | (1<<ADPS0));

   // clear ADCSRB. page 251
   ADCSRB = 0x00;
}

uint16_t adc_read(uint8_t channel) {
   uint16_t result = 1;

   // clear mux bits
   ADMUX &= ~0x1F;

   // set mux bits
   ADMUX |= (channel & 0x7);

   // initiate conversion
   ADCSRA |= (1 << ADSC);

   // wait for conversion to finish
   while( ADCSRA & (1 << ADSC) );

   result = ADC;

   return result;
}

int main() {

   adc_init();

   uint16_t adc;

   DDRD |= 1;
   //PORTD |= 1;

   uint16_t i, j, k, l;
   l=0;
   
   while(1) {
      adc = adc_read(0);

      adc /= 128;
      // output
      for( j=0; j<adc; j++ ) {
         PORTD |= 1;
         // delay loop
         for( i=0; i<20000; i++ );

         PORTD &= ~0x1;
         // delay loop
         for( i=0; i<20000; i++ );
      }
      //PORTD &= ~1;
      for( i=0; i<30000; i++ );
      PORTD |= 1;
      for( i=0; i<60000; i++ );
      for( i=0; i<30000; i++ );
      PORTD &= ~1;
      for( i=0; i<30000; i++ );
   }
}
