/* Name: write.h
 * Author: Grant Gunnison
 *
 * Library that can be used to write to the cube.
 * Includes a function that takes in an array for a horizontal level and a 
 * integer for a vertical level and displays it on the cube.
 */

#ifndef writeCube
#define writeCube
#include <avr/io.h>


int writeLevelToCube(int array[], int index);

//Pinout from the Atmega32 to the SN74HC595
// PB0 = SRCLK 0x00
// PB1 = RCLK 0x02
// PB2 = SER 0x04
// PB3 = OE 0x08


//Defining macros to be used throughout code.
#define bit_set(p,m) ((p) |= (m));
#define bit_clear(p,m) ((p) &= ~(m));
#define bit_flip(p,m) ((p) ^= (1<<m));

/* Inputs: 
* Array: 72 bit representation of the cube, 64 bits for individual columns, 8 bits for the layers.
* 		 The last 8 bits of the 72 control the layers, in ascending order. Bit 65 is the bottom, 72 is the top.
*
* Index: This function takes in an index to display a single bit and passes its index++ along.
*
* This function takes in a array of a single layer of the cube and will display it. 
* More than a single level can be displayed at once, but only a single layer is individually addressable.
* 
* This function is meant to be used in an interrupt that will determine its frequency, as such this function
* only writes a single bit per call starting from the beginning of the array, passing its index along for the 
* next interrupt cycle
*
*  Outputs: index++
*/

int writeLevelToCube(int array[], int index){
	bit_flip(PORTB, 0x00);				// Flip SRCLK to create clock signal for SN74HC595
	bit_clear(PORTB, 0x02);				// We want to keep RCLK low until we have shifted the entire byte in
	if (bit_is_set(PORTB, 0)){			// Only when the clock is high do we want to move information in.
		if (index == 72){				// Toggle RCLK once byte is moved in.
			bit_set(PORTB, 0x02);
			index = 0;
		}
		if (array[index]){		// write the bit to PIN 2 that is up next in our byte
			bit_set(PORTB, 0x04);		// if the bit is a 1 use this.
		}
		else{
			bit_clear(PORTB, 0x04);		// if the bit is a 0 use this.
		}	
		index++;						
	}
	return index;
}

void writehello(){

}



#endif
