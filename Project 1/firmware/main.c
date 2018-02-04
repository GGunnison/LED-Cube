/* Name: main.c
 * Author: Grant Gunnison
 * with help from Newbiehack.com
 */

#include <avr/io.h>

int main(void)
{
    /* insert your hardware initialization here */
    //initialize port for LED's 
    DDRB = 0b00000001;
    //PORTB = 0b00000000;
    TCCR1B = 0b00000001;
    while(1){
    	
    	if (TCNT1 > 40000){
    		TCNT1 = 0;
    		PORTB ^= 1<< PINB0;
    	}

    }
    return 0;
}

