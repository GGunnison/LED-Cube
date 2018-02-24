/* Name: main.c
 * Author: Grant Gunnison
 *
 * This is the first test script to interface the Atmega32 with a SN74HC595 chip.
 * This test code pushes one byte to a single 8-bit shift register and displays that
 * byte on the output pins of the chip. 
 */

#include <avr/io.h>
#include <avr/interrupt.h>


//Defining macros to be used throughout code.
#define bit_set(p,m) ((p) |= (m));
#define bit_clear(p,m) ((p) &= ~(m));
#define bit_flip(p,m) ((p) ^= (1<<m));
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m));


//Defining global variables that are used in script
int counter = 0;
int byte[] = {1,0,1,0,1,0,1,0}; //This is the byte to be displayed


//Pinout from the Atmega32 to the SN74HC595
	// PB0 = SRCLK 0x00
	// PB1 = RCLK 0x02
	// PB2 = SER 0x04
	// PB3 = OE 0x08
	

int main(void)
{
	
	DDRB |= 0x0F;						// Initialize output pins
	TCCR1B = (1<<CS10) | (1<<WGM12);	// Prescaling 8x and setting up Waveform generation mode
	TIMSK |= 1<<OCIE1A;					// Initializing the compare A register
	OCR1A = 500;						// Interrupt timer count
	bit_clear(PORTB, 0x08);				// Enable OE for the SN74HC595
	sei();								// Enable global interrupts

	//Do nothing outside of the interrupt
    while(1){
    }

}

ISR(TIMER1_COMPA_vect){					// Interrupt vector corresponding to OCIE1A 
	bit_flip(PORTB, 0x00);				// Flip SRCLK to create clock signal for SN74HC595
	bit_clear(PORTB, 0x02);				// We want to keep RCLK low until we have shifted the entire byte in
	if (bit_is_set(PORTB, 0)){			// Only when the clock is high do we want to move information in.
		if (counter == 8){				// Toggle RCLK once byte is moved in.
			bit_set(PORTB, 0x02);
			counter = 0;
		}
		if (byte[counter]){				// write the bit to PIN 2 that is up next in our byte
			bit_set(PORTB, 0x04);		// if the bit is a 1 use this.
		}
		else{
			bit_clear(PORTB, 0x04);		// if the bit is a 0 use this.
		}
		counter++;						// iterate the count to keep track of where we are in our byte.
	}
}
