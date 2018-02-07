/* Name: main.c
 * Author: Grant Gunnison	
 *
 * This is the first test script to enable an interrupt using the atmega32
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>



int main(void)
{
	
	DDRB |= 1<<PB0;						//Set pin0 to an output pin
	TCCR1B = (1<<CS10) | (1<<WGM12); 	//Set counter to use internal oscillator and CTC
	TIMSK |= 1<<OCIE1A;					//Match Compare A interrupt enable
	OCR1A = 5000;						//set counter value to interrupt at
	sei();								//global interrupt enable

    while(1){
    }

}

ISR(TIMER1_COMPA_vect){					// interrupt vector compare A
	PORTB ^= 1<<PB0;					// flip pin 0
}


