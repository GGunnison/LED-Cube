/* Name: main.c
 * Author: Grant Gunnison
 * with help from Newbiehack.com
 *
 *
 *First test script to get an led to light up from pin 1 on the Atmega32
 */

#include <avr/io.h>

int main(void)
{
    DDRB |= 0x01; //initialize pin 1 as an output port
    TCCR1B = 0x01; //set cs10 pin for timer to use internal oscilator with no prescaling
    while(1){
    	
    	if (TCNT1 > 40000){ //set cycle count 
    		TCNT1 = 0;      //reset counter
    		PORTB ^= 1<< PINB0; //flip bit to blink led
    	}

    }
    return 0;
}

