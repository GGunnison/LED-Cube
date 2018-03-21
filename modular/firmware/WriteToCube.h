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
#include <ctype.h>

//Prototypes
unsigned char writeLevelToCube(volatile unsigned char array[], unsigned char index);
void writeLevel(volatile unsigned char array[9], unsigned char numRows, unsigned char numCol);
void writeFullCube(volatile unsigned char array[8][9], unsigned char numRows, unsigned char numCol);
int writePhrase(volatile unsigned char phrase[], float cubeWriteTime, float displayTime, float percentage, volatile int counter, int index, volatile int libIndex);

//Defining macros to be used throughout code.
#define bit_set(p,m) ((p) |= (m));
#define bit_clear(p,m) ((p) &= ~(m));
#define bit_flip(p,m) ((p) ^= (1<<m));

//Pin addresses for data pins
#define SCLK    0x00 //PB0
#define SCLKPIN 0x01 //PB0
#define RCLK    0x02 //PB1
#define DATA    0x04 //PB2
#define OE      0x08 //PB3


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

void writeLevel(volatile unsigned char array[9], unsigned char numRows, unsigned char numCol){ 
	
	unsigned char numElements = numCol*numRows;
	char index;
	//We want to clear the latch before we start shift in data.
	bit_clear(PORTB, RCLK); 
	
	if ((numElements != 72) || (numRows != 8)){
		// throw error
		// Compiler does not optimize for loops using variables vs. a constant

	}
	
	for(index=0; index<=72; index++){
		bit_set(PORTB, SCLKPIN);				// Flip SRCLK to create clock signal for SN74HC595
		if (bit_is_set(PORTB, SCLK)){			// Only when the clock is high do we want to move information in.
			if (index == 72){				
				bit_set(PORTB, RCLK);		// Toggle RCLK once byte is moved in.
			}
			if ((array[(index/8)] & (1 << index % 8)) != 0){		// write the bit to PIN 2 that is up next in our byte
				bit_set(PORTB, DATA);		// if the bit is a 1 use this.
			}
			else{
				bit_clear(PORTB, DATA);	     //check if the bit is a 0 use this.
			}							
		}bit_clear(PORTB, SCLKPIN);
		
	}bit_clear(PORTB, RCLK);
}
/*
Inputs: array[8][9]: model of the cube in a nested array structure
		numRows:     number of rows
		numCol:      number of columns

This function takes in a model of the cube and displays it once to the cube. The layers are
displayed starting from the topmost layer.

The output enable for the shift registers is deactivated after each complete cycle so that
if there is any amount of delay between calls of this function the last layer displayed does
not receive a larger amount of time to display than any of the others. 
*/

void writeFullCube(volatile unsigned char array[8][9], unsigned char numRows, unsigned char numCol){
	bit_clear(PORTB, OE);
	char index;
	for (index = 0; index<numRows; index++){
		writeLevel(array[index], numRows, numCol);
	}
	bit_set(PORTB, OE);

}
/*
BROKEN METHOD, used an interrupt instead, do not attempt to use this method.

The write Phrase method takes in a phrase, time for how long to display a character, and a cycleFlag to know
how many times each letter has been shown. It provides the index in the character library stored in memory. Using
ASCII codes. ASCII codes for capital letters are sequential with A starting at 63 and Z ending with 88. 

This function removes spaces, but allows them to be in the phrase for readability.
*/

int writePhrase(volatile unsigned char phrase[], float cubeWriteTime, float displayTime, float percentage, volatile int counter, int index, volatile int libIndex){
	
	
	float cycleTime = cubeWriteTime/(percentage/100);
	int numCycles = displayTime/cycleTime;
	int letter;


	if(counter == numCycles){
		counter = 0;
		letter = (phrase[index]);
		while (letter == 32){
			index= index+1;
			letter = (phrase[index]);
		}
		index = (index+1)%sizeof(phrase);
		libIndex = (toupper(letter)-64);

		}
	return index;
	}



#endif
