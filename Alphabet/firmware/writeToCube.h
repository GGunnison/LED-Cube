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


unsigned char writeLevelToCube(volatile unsigned char array[], unsigned char index);
void writeFullCube();

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
* Array: 9 byte representation of the cube, 8 bytes for lines in the layer and 1 byte to indicate the level.
* 		 Bit 65 is the top, 72 is the bottom.
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

unsigned char writeLevelToCube(volatile unsigned char array[], volatile unsigned char index){
	bit_flip(PORTB, 0x00);				// Flip SRCLK to create clock signal for SN74HC595
	bit_clear(PORTB, 0x02);				// We want to keep RCLK low until we have shifted the entire byte in
	if (bit_is_set(PORTB, 0)){			// Only when the clock is high do we want to move information in.
		if (index == 72){				
			bit_set(PORTB, 0x02);		// Toggle RCLK once byte is moved in.
			index = 0;
		}
		if ((array[(index/8)] & (1 << index % 8)) != 0){		// write the bit to PIN 2 that is up next in our byte
			bit_set(PORTB, 0x04);		// if the bit is a 1 use this.
		}
		else{
			bit_clear(PORTB, 0x04);	     //check if the bit is a 0 use this.
		}	
		index++;						
	}
	return index;
}	
/*

This function writes a single level to the cube at a time. This function is intended 
to be self contained and used once per interrupt.

Input: Single dimensional array representing a single level of the cube.

Output: boolean style flag representing the completion of the method. 




*/


unsigned char writeLevel(volatile unsigned char array[], char sClkPin, char rClkPin, char dataPin, char OEPin){
	unsigned char numElements = sizeof(array);
	bit_clear(PORTB, rClkPin);
	bit_clear(PORTB, OEPin);
	char cubeWidth = 8;
	char complete = 1;
	char index;

	for(index=0; index<(2*numElements); index++){
		bit_flip(PORTB, 0x00);				// Flip SRCLK to create clock signal for SN74HC595
		if (bit_is_set(PORTB, 0)){			// Only when the clock is high do we want to move information in.
			if (index == 143){				
				bit_set(PORTB, 0x02);		// Toggle RCLK once byte is moved in.
			}
			if ((array[(index/8)] & (1 << index % 8)) != 0){		// write the bit to PIN 2 that is up next in our byte
				bit_set(PORTB, 0x04);		// if the bit is a 1 use this.
			}
			else{
				bit_clear(PORTB, 0x04);	     //check if the bit is a 0 use this.
			}							
		}

	// 	bit_set(PORTB, sClkPin);
	// 	if ((array[(index/cubeWidth)] & (1 << index % cubeWidth)) != 0){		// write the bit to PIN 2 that is up next in our byte
	// 		bit_set(PORTB, dataPin);		// if the bit is a 1 use this.
	// 	}
	// 	else{
	// 		bit_clear(PORTB, dataPin);	     //check if the bit is a 0 use this.
	// 	}
	// 	bit_clear(PORTB, sClkPin);
		
	// }
	}

		
	return complete;
}


// void writeFullCube(volatile unsigned char array[8][9]){
// 	char numLevels = sizeof(array);
// 	char elements = sizeof(array[0])
// 	char level = 7;
// 	char pinCounter = 0;


// 	for(unsigned char i=0; i<numLevels; i++){
// 		(((letterLib[(word[letterIndex]-65)][level][(counter/8)]) & (1 << counter % 8)) != 0)
// 		for(unsigned char j=0; j<elements; j++){
// 			bit_set(PORTB, 0);

// 		}
// 	}

// 	bit_flip(PORTB, 0x00);				// Flip SRCLK to create clock signal for SN74HC595
// 	if (bit_is_set(PORTB, 0)){							// Only when the clock is high do we want to move information in.
		
// 		if (((letterLib[(word[letterIndex]-65)][level][(counter/8)]) & (1 << counter % 8)) != 0){		// write the bit to PIN 2 that is up next in our byte
// 			bit_set(PORTB, 0x04);		// if the bit is a 1 use this.
// 		}
// 		else{
// 			bit_clear(PORTB, 0x04);	     //check if the bit is a 0 use this.
// 		}
		
// 	}else{
// 		if (counter == 72){				
// 			bit_set(PORTB, 0x02);		// Toggle RCLK once byte is moved in.
// 			counter = 0;
// 			level++;
// 			if (level == 8){
// 				level = 0;
// 				cycleFlag = 1;
// 			}
// 		}	
// 		counter++;
// 		bit_clear(PORTB, 0x02);				// We want to keep RCLK low until we have shifted the entire byte in

// 	}

// }



#endif
