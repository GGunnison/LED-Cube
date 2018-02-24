/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>

int main(void)
{

	USRC &= ~(1<<UMSEL);
	UBBR = 25;
	UBBRH = (unsigned char) (UBBR >> 8);
	UBBRL = (unsigned char) UBBR;	

	UCSRB = (1<<RXEN) | (1<< TXEN);
	UCSRC = (1 << USBS) | (3<< UCSZ0);

	while(1){
		while (!(UCSRA & (1<< UDRE)) );
		UDR = 0b11110000;
    /* insert your hardware initialization here */
    
}
