/* Name: main.c
 * Author: Grant Gunnison
 *
 * This program interfaces with the cube through a UART serial connection
 * It allows the user to print a custom phrase to the cube and adjust the
 * speed and brightness of the output. 
 *
 * Improvements: extend the usable memory space to hold more characters. This version
 * currently does not have a full alphabet etc.
 */
#include "writeToCube.h"
#include "CubeData.h"
#include "UARTLib.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h> 
#include <ctype.h>
#include <string.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>



//Defining macros to be used throughout code.
#define bit_set(p,m) ((p) |= (m));
#define bit_clear(p,m) ((p) &= ~(m));
#define bit_flip(p,m) ((p) ^= (1<<m));


//Prototypes
void setbrightness(int percentage);
void init16bitTimer1(volatile float displayTime);
void setupMCUIO(char portA, char portB, char portC, char portD);
void initTimer0(int percentage);
void uart_transmit(unsigned char data);
unsigned char uart_receive();
void sendMessage(const unsigned char message[], unsigned int size, int start);
void sendLongMessage(volatile char message[], unsigned int size, int start);
void sendPROGMessage(const unsigned char message[], unsigned int size);
char isValid(volatile unsigned char character);

//GLOBAL VARIABLES
//Words to intialize the GPIO pins on the MCU
#define AIO 0x00
#define BIO 0x0F
#define CIO 0x00
#define DIO 0x00

//Pins used from the Atmega32 to the SN74HC595
	// PB0 = SRCLK 0x00
	// PB1 = RCLK 0x02
	// PB2 = SER 0x04
	// PB3 = OE 0x08

//Variable for Timer 1, this sets speed of interrupt, which also set the brightness of the LED's
volatile int brightness = 100;
char displayBrightnessBuffer[3];
volatile unsigned char brightnessBufferIndex = 0;
//Phrase to be displayed
volatile char displayPhrase[40] = "_";



//Speed of each frame displayed on cube
volatile float displayTime = 1000;
volatile char displayTimeBuffer[4];
volatile unsigned char timeBufferIndex = 0;



//Variables for phrase interrupt
volatile int libIndex = 0;
// volatile int interruptCycles = 61; // F_CPU/262,144 = 1024 prescaler * 255 for 1 second display
volatile int phraseIndex = 0;
volatile int letter;
volatile unsigned char numRows = 8;


//Variables for UART communication
static int UBBRValue = 103;
volatile unsigned char ReceivedByte;
volatile unsigned char commandFlag = 0;
volatile unsigned char writeFlag = 0;
volatile unsigned char brightnessFlag = 0;
volatile unsigned char timeFlag = 0;
volatile int bufferIndex = 1;

const uint8_t EEMEM special6 = 0x41;


int main(void)
{
	setupMCUIO(AIO, BIO, CIO, DIO);
	initTimer0(brightness);
	init16bitTimer1(displayTime);
	
	USART_Init(UBBRValue);
	UCSRB |= (1 << RXCIE);
	sei();	
	sendPROGMessage(startPhrase, sizeof(startPhrase));

	//Do nothing outside of the interrupt
    while(1){

	    }
}

// ISR(UART COMMUNICATION)
ISR(TIMER0_COMP_vect){

	writeFullCube(letterLib[libIndex], numRows, sizeof(letterLib[0][0]));
}
ISR(TIMER1_COMPA_vect){
	letter = (displayPhrase[phraseIndex]);
	phraseIndex = (phraseIndex+1)%(bufferIndex);
	if((letter =='_') || (letter == 0x20)){ // Space and base of display phrase
		letter = 64;
		libIndex = (letter-64);
		}
	else if (letter == 0x21){ //Exclamation mark
		letter = 92;
		libIndex = (letter-64);
	}
	else if (letter == 0x3F){ // Question mark
		letter = 91;
		libIndex = (letter-64);
	}
	else if (letter == 0x2D){ // Dash
		letter = 105;
		
		libIndex = (letter-64);
	}
	else if (letter == 0x3D){ //Equals sign
		letter = 104;
		
		libIndex = (letter-64);
	}
	else if (letter == 0x3A){ //Colon
		letter = 103;
		
		libIndex = (letter-64);
	}
	else if (letter == 0x2E){ //Period
		letter = 106;
		
		libIndex = (letter-64);
	}
	else if((letter >= 0x30) && (letter <=0x39)){ // Integers
		letter += 0x2D;
		libIndex = (letter-64);
	}
	else if ((letter >=0x41 && letter <=0x5A) || (letter>= 0x61 && letter <=0x7A)){
		libIndex = (toupper(letter)-64);
	}

}

ISR(USART_RXC_vect){
	ReceivedByte = UDR;
	
	if (ReceivedByte == ' '){                     //This is done to translate ASCII code for a space to the position in memory 
		ReceivedByte = 64;  					   //That the data for a space is located.
	}
	else if ((ReceivedByte == '-') && !(commandFlag)){ // Symbol: - to set command flag
		commandFlag = 1; 

	}
	else if ((ReceivedByte == 'C') && (commandFlag)){ 					  // Symbol:C clear the existing word from the cube. 
		bufferIndex = 1;
		sendPROGMessage(clearCube, sizeof(clearCube));
	}
	else if((ReceivedByte == 'H') && (commandFlag)){					  // Symbol:H to see instructions & available commands
		sendPROGMessage(commands, sizeof(commands));
	}

	else if ((ReceivedByte == 'W') && (commandFlag)){ // Symbol:W to set phrase to display
		writeFlag = 1;
		bufferIndex = 1;

		//set all other flags to 0
		commandFlag = 0;
	}
	else if ((ReceivedByte == 'T') && (commandFlag)){ // Symbol:T to set phrase display time
		//set the phrase to be displayed
		timeFlag = 1;
		timeBufferIndex = 0;

		//set all other flags to 0
		commandFlag = 0;
		
	}
	else if ((ReceivedByte == 'B') && (commandFlag)){ // Symbol:B to set brightness
		//set the phrase to be displayed
		brightnessFlag = 1;
		brightnessBufferIndex = 0;
		
		//set all other flags to 0
		commandFlag = 0;
	}
	else if (writeFlag){
		//Skip any spaces preceeding a word.
		if ((ReceivedByte == 64) && (bufferIndex == 1)){
			return;	
		}
		//Terminal sends end byte to declare end of transmission 
		//
		if (ReceivedByte == 0x0D){
			sendPROGMessage(writeEcho, sizeof(writeEcho));
			sendLongMessage(displayPhrase, bufferIndex, 1);
			writeFlag = 0;
			return;
		}
		if(isValid(ReceivedByte)){
			displayPhrase[bufferIndex] = toupper(ReceivedByte);
			bufferIndex++;
		}else{
			sendPROGMessage(invalidCharacter, sizeof(invalidCharacter));
			UDR = ReceivedByte;
			writeFlag = 0;
			bufferIndex = 1;
		}

	}
	else if (timeFlag){

		if (ReceivedByte == 64){ //Skip any spaces
			return;			
		}
		//Terminal sends end byte to declare end of transmission 
		if(ReceivedByte == 0x0D){
			//If an user enters a value with less digits than something entered before 
			//we only want to send back what was sent this time around
			char timeBufferTrunc[4];
			memcpy(timeBufferTrunc, displayTimeBuffer, timeBufferIndex);
			
			//Must convert to an integer to handle bounds. Send back value to terminal
			long temp = atoi(timeBufferTrunc);
			sendPROGMessage(setTime, sizeof(setTime));
			sendLongMessage(timeBufferTrunc, timeBufferIndex,0);
			sendPROGMessage(timeEnding, sizeof(timeEnding));
			
			//Make sure to handle bounds of hardware
			if ((temp < 4000) && (temp > 10)){
				displayTime = temp;
				init16bitTimer1(displayTime);

			}else{
				sendPROGMessage(timeError, sizeof(timeError));
				
			}
			timeFlag = 0;
			return;	
		}
		//Add to buffer any character sent, increment buffer index.
		displayTimeBuffer[timeBufferIndex] = ReceivedByte;
		timeBufferIndex++;
	}
	else if(brightnessFlag){
		if (ReceivedByte == 64){ //Skip any spaces
			return;			
		}
		//Terminal sends end byte to declare end of transmission 
		if(ReceivedByte == 0x0D){
			//If an user enters a value with less digits than something entered before 
			//we only want to send back what was sent this time around
			char brightnessBufferTrunc[3];
			memcpy(brightnessBufferTrunc, displayBrightnessBuffer, brightnessBufferIndex);
			
			//Must convert to an integer to handle bounds. Send back value to terminal
			int temp = atoi(brightnessBufferTrunc);

			//Make sure to handle bounds of hardware
			if ((temp > 100) || (temp < 14)){
				sendPROGMessage(brightnessError, sizeof(brightnessError));				
			}else{

				sendPROGMessage(setBrightness, sizeof(setBrightness));
				sendLongMessage(brightnessBufferTrunc, brightnessBufferIndex,0);
				sendPROGMessage(percentSign,sizeof(percentSign));
			
				brightness = temp;
				initTimer0(brightness);
			}
			brightnessFlag = 0;
			return;
		}
		displayBrightnessBuffer[brightnessBufferIndex] = ReceivedByte;
		brightnessBufferIndex++;
	}
}
/* BRIGHTNESS
	Each cycle to write a full cube takes approximately 2ms, which provides a 500Hz output frequency.
	To control brightness the amount of time in between each cycle is increased, by increasing the 
	length of the interrupt.

	With a prescale of 256, 135 is loaded into the OCR1A register to provide a 2.2ms interrupt using a
	16Mhz clock.

	Given that the OCR1A register can only hold up to 65000 in its register, the resolution of 
	brightness will be better than 1%. However, to still maintain persistence of vision, the lowest
	the brightness can be is around 10% or 50Hz.
*/

void setbrightness(int percentage){
	OCR0 = 34/(percentage/100.0);		// Interrupt timer count
	return;
}
/*
	Setup Timer 0 with a 1024 prescaler and 255 (8-bit) count interrupt cycle
	TIMSK must be OR'd to make sure to not mess with other timers.
*/

void initTimer0(int percentage){
	TCCR0 = (1<<CS02) | (1<<CS00) | (1 << WGM01);
	TIMSK |= (1<<OCIE0);

	setbrightness(percentage);
	return;
}
/*
	Initializes the 16 bit timer on the Atmega32. 
	1024x Prescaling is set
	WGM1 is set to interrupt when the OCR1A register hits zero.
	OCIE1A register is set to compare at the A interrupt vector

*/
void init16bitTimer1(volatile float displayTime){
	TCCR1B = (1<<CS12) | (1<<CS10) | (1<<WGM12);	
	TIMSK |= (1<<OCIE1A);					        // Initializing the compare A register
	OCR1A = lrint(15625*(displayTime/1000)); 		// set to 1 second with displayTime value of 1
}
/*
	Initialize all input/output pins on the Atmega.
	0 initializes pin as an input, 1 as an output.
*/

void setupMCUIO(char portA, char portB, char portC, char portD){
	
	DDRA |= portA;			// PortA
	DDRB |= portB;			// PortB
	DDRC |= portC;			// PortC
	DDRD |= portD;			// PortD
}
/*
This function will echo back via serial connection the array provided to it.
Size of array is required since the objects are not reflexive.
*/
void sendMessage(const unsigned char message[], unsigned int size, int start){
	int i;
	for (i = start; i<(size); i++){
		if (message[i] == 64){                   //This is done to translate the memory code for a space to the ASCII Code 
			UDR = 0x20;  				   //ASCII Code for a space
		}
		else{
			UDR = message[i];
		}
		_delay_us(1500); //Delay provided because of datatransfer & lower interrupt priorty.
						 //Timer 0 interrupt can last as long as 2ms.
		
	}
}
/*
This function will echo back via serial connection the array provided to it.
Size of array is required since the objects are not reflexive.
This method is used for volatile objects
*/
void sendLongMessage(volatile char message[], unsigned int size, int start){
	int i;
	for (i = start; i<(size); i++){
		if (message[i] == 64){                   //This is done to translate the memory code for a space to the ASCII Code 
			message[i] = 0x20;  				   //ASCII Code for a space
		}
		UDR = message[i];
		_delay_us(1500); //Delay provided because of datatransfer & lower interrupt priorty.
						 //Timer 0 interrupt can last as long as 2ms.
		
	}
}

void sendPROGMessage(const unsigned char message[], unsigned int size){
	int i;
	for (i = 0; i<(size); i++){
		if (message[i] == 64){                   //This is done to translate the memory code for a space to the ASCII Code 
			UDR = 0x20;  				   //ASCII Code for a space
		}
		else{
			UDR = pgm_read_byte(&(message[i]));
		}
		_delay_us(1500); //Delay provided because of datatransfer & lower interrupt priorty.
						 //Timer 0 interrupt can last as long as 2ms.
		
	}				 //Timer 0 interrupt can last as long as 2ms.
}

char isValid(volatile unsigned char character){
	
	if ((toupper(character) == 64) || (toupper(character) == 0x21) || (toupper(character) == 0x2E) ||(toupper(character) == 0x3F) 
		|| (toupper(character) == 0x2D) || (toupper(character) == 0x3D) || (toupper(character) == 0x3A) ||
		((toupper(character) >= 0x30) && (toupper(character) <=0x39)) || (toupper(character) >=0x41 && toupper(character) <=0x5A)){
		return 1;
	}else{
		return 0;
	}
}





