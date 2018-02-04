/* Name: main.c
 * Author: Grant Gunnison	
 */

#include <avr/io.h>
#include <avr/interrupt.h>


int main(void)
{
	sei();
	DDRB |= 1<<PB0;
	PORTB ^= 1<<PB0;
	TCCR1B = (1<<CS10) | (1<<WGM12);
	TIMSK |= 1<<OCIE1A;
	OCR1A = 25;

    while(1){
    }

}

ISR(TIMER1_COMPA_vect){
	PORTB ^= 1<<PB0;
}


