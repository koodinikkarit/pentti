/*
 * pentti.c
 *
 * Created: 6.2.2017 10.14.36
 * Author : jaska
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>  


int main(void)
{
    
	  TCCR0A = 0;// set entire TCCR2A register to 0
  TCCR0B = 0;// same for TCCR2B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 4;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

    while (1) 
    {
    }
}

ISR(TIMER0_COMPA_vect){//timer0 interrupt 2kHz toggles pin 8
	volatile int a = 5;
	a += 30;
}

