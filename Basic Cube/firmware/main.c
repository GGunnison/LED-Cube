/* Name: main.c
 * Author: Grant Gunnison
 *
 * This is the first test script to interface the Atmega32 with a SN74HC595 chip.
 * This test code pushes one byte to a single 8-bit shift register and displays that
 * byte on the output pins of the chip. 
 */
#include "writeToCube.h"
#include "CubeData.h"
#include <avr/io.h>
#include <avr/interrupt.h>


#ifndef F_CPU
#define F_CPU 160000000UL
#endif



//Defining macros to be used throughout code.
#define bit_set(p,m) ((p) |= (m));
#define bit_clear(p,m) ((p) &= ~(m));
#define bit_flip(p,m) ((p) ^= (1<<m));


//Defining global variables that are used in script
volatile unsigned char counter = 0;
volatile unsigned char level = 0;

//Pinout from the Atmega32 to the SN74HC595
	// PB0 = SRCLK 0x00
	// PB1 = RCLK 0x02
	// PB2 = SER 0x04
	// PB3 = OE 0x05	

int main(void)
{
	
	DDRB |= 0x0F;						// Initialize output pins
	TCCR1B = (1<<CS10) | (1<<WGM12);	// No prescaling and setting up Waveform generation mode
	TIMSK |= 1<<OCIE1A;					// Initializing the compare A register
	OCR1A = 150;							// Interrupt timer count
	bit_clear(PORTB, 0x08);				// Enable OE for the SN74HC595
	sei();								// Enable global interrupts

	//Do nothing outside of the interrupt
    while(1){
    	if (bit_is_set(PORTB, 0)){
			if(counter == 72){
				level++;
				if (level == 8){
					level = 0;
				}		
			}
		}
    }
}	


ISR(TIMER1_COMPA_vect){					// Interrupt vector corresponding to OCIE1A 
	counter = writeLevelToCube(A[level], counter);
	}


