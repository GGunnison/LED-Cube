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
volatile unsigned char cycleCount = 0;
volatile unsigned char cycleFlag =0;
volatile unsigned char letterIndex = 0;
unsigned char frequency = 16;
unsigned char time = 1;

//Pinout from the Atmega32 to the SN74HC595
	// PB0 = SRCLK 0x00
	// PB1 = RCLK 0x02
	// PB2 = SER 0x04
	// PB3 = OE 0x08	

int main(void)
{
	
	DDRB |= 0x0F;						// Initialize output pins
	TCCR1B = (1<<CS10) | (1<<WGM12);	// No prescaling and setting up Waveform generation mode
	TIMSK |= 1<<OCIE1A;					// Initializing the compare A register
	OCR1A = 100;							// Interrupt timer count
	bit_set(PORTB, 0x00);				// Enable OE for the SN74HC595
	sei();								// Enable global interrupts

	//Do nothing outside of the interrupt
    while(1){
    	if (cycleFlag){
    		cycleCount++;
    		cycleFlag = 0;
    	}if (cycleCount == (frequency*time)){
    		letterIndex = (letterIndex +1)%26;
    		cycleCount = 0;
    	}

    }
}	


ISR(TIMER1_COMPA_vect){					// Interrupt vector corresponding to OCIE1A 
	
	bit_flip(PORTB, 0x00);				// Flip SRCLK to create clock signal for SN74HC595
	if (bit_is_set(PORTB, 0)){							// Only when the clock is high do we want to move information in.
		
		if (((letterLib[letterIndex][level][(counter/8)]) & (1 << counter % 8)) != 0){		// write the bit to PIN 2 that is up next in our byte
			bit_set(PORTB, 0x04);		// if the bit is a 1 use this.
		}
		else{
			bit_clear(PORTB, 0x04);	     //check if the bit is a 0 use this.
		}
		
	}else{
		if (counter == 72){				
			bit_set(PORTB, 0x02);		// Toggle RCLK once byte is moved in.
			counter = 0;
			level++;
			if (level == 8){
				level = 0;
				cycleFlag = 1;
			}
		}	
		counter++;
		bit_clear(PORTB, 0x02);				// We want to keep RCLK low until we have shifted the entire byte in

	}
}


