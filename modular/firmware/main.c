/* Name: main.c
 * Author: Grant Gunnison
 *
 * This is the first test script to interface the Atmega32 with a SN74HC595 chip.
 * This test code pushes one byte to a single 8-bit shift register and displays that
 * byte on the output pins of the chip. 
 */
#include "writeToCube.h"
#include "CubeData.h"
#include "UARTLib.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h> 
#include <ctype.h>



//Defining macros to be used throughout code.
#define bit_set(p,m) ((p) |= (m));
#define bit_clear(p,m) ((p) &= ~(m));
#define bit_flip(p,m) ((p) ^= (1<<m));


//Prototypes
void setbrightness(float percentage);
void init16bitTimer1(float percentage);
void setupMCUIO(char portA, char portB, char portC, char portD);
void initTimer0(void);
void uart_transmit(unsigned char data);
unsigned char uart_receive();



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
float brightness = 20;
//Phrase to be displayed
volatile unsigned char phrase[] = "Im alive";
//Speed of each frame displayed on cube
volatile float displayTime = 1;


//Variables for phrase interrupt
volatile int counter = 0;
volatile int libIndex = 0;
volatile int interruptCycles = 61; // F_CPU/262,144 = 1024 prescaler * 255 for 1 second display
volatile int phraseIndex = 0;
volatile int letter;
volatile unsigned char numRows = 8;

//Variables for UART communication
static int baudrate = 25;
static char doubleSpeed = 0; // 1 if active
static int dataSize = 8; // 1 byte
static char parity = 0; // no parity bit
static char stopBitNum = 1;
char ReceivedByte;

int main(void)
{
	
	setupMCUIO(AIO, BIO, CIO, DIO);
	init16bitTimer1(brightness);
	initTimer0();
	USART_Init(103);



	//Do nothing outside of the interrupt
    while(1){
    	ReceivedByte = uart_receive();
    	uart_transmit(ReceivedByte);
    	
    	
	    }
}
// ISR(UART COMMUNICATION)

//Interrupt to write each layer 	
ISR(TIMER1_COMPA_vect){
	writeFullCube(letterLib[libIndex], numRows, sizeof(letterLib[0][0]));
}

//Interrupt to change the frame being displayed on the cube. 
ISR(TIMER0_COMP_vect){
	counter++;
	int numCycles = floor(displayTime*interruptCycles);
	if(counter == numCycles){
		counter = 0;
		letter = (phrase[phraseIndex]);
		while (letter == 32){
			phraseIndex++;
			letter = (phrase[phraseIndex]);
		}
		phraseIndex = (phraseIndex+1)%sizeof(phrase);
		libIndex = (toupper(letter)-64);
	}
}


/* BRIGHTNESS
	Each cycle to write a full cube takes approximately 2ms, which provides a 500Hz output frequency.
	To control brightness the amount of time in between each cycle is increased, by increasing the 
	length of the interrupt.

	With a prescale of 256, 125 is loaded into the OCR1A register to provide a 2ms interrupt using a
	16Mhz clock.

	Given that the OCR1A register can only hold up to 65000 in its register, the resolution of 
	brightness will be better than 1%. However, to still maintain persistence of vision, the lowest
	the brightness can be is around 10% or 50Hz.
*/

void setbrightness(float percentage){
	OCR1A = 500/(percentage/100);		// Interrupt timer count
}

/*
	Initializes the 16 bit timer on the Atmega32. 
	64x Prescaling is set
	WGM1 is set to interrupt when the OCR1A register hits zero.
	OCIE1A register is set to compare at the A interrupt vector

*/
void init16bitTimer1(float percentage){
	TCCR1B = (3<<CS10) | (1<<WGM12);	
	TIMSK |= (1<<OCIE1A);					// Initializing the compare A register

	setbrightness(percentage);
	sei();											// Enable global interrupts

}

/*
	Setup Timer 0 with a 1024 prescaler and 255 (8-bit) count interrupt cycle
	TIMSK must be OR'd to make sure to not mess with other timers.
*/

void initTimer0(void){
	TCCR0 = (1 <<CS00) | (1<<CS02) | (1 << WGM01);
	TIMSK |= (1<<OCIE0);
	OCR0 = 255;

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


// /* BRIGHTNESS
// 	Each cycle to write a full cube takes approximately 2ms, which provides a 500Hz output frequency.
// 	To control brightness the amount of time in between each cycle is increased, by increasing the 
// 	length of the interrupt.

// 	With a prescale of 64, 500 is loaded into the OCR1A register to provide a 2ms interrupt using a
// 	16Mhz clock.

// 	Given that the OCR1A register can only hold up to 65000 in its register, the resolution of 
// 	brightness will be better than 1%. However, to still maintain persistence of vision, the lowest
// 	the brightness can be is around 10% or 50Hz.
// */

// void setbrightness(float percentage){
// 	OCR1A = 500/(percentage/100);		// Interrupt timer count
// }

// /*
// 	Initializes the 16 bit timer on the Atmega32. 
// 	64x Prescaling is set
// 	WGM1 is set to interrupt when the OCR1A register hits zero.
// 	OCIE1A register is set to compare at the A interrupt vector

// */
// void init16bitTimer1(float percentage){
// 	TCCR1B = (3<<CS10) | (1<<WGM12);	
// 	TIMSK |= (1<<OCIE1A);					// Initializing the compare A register

// 	setbrightness(percentage);
// 	sei();											// Enable global interrupts

// }

// /*
// 	Setup Timer 0 with a 1024 prescaler and 255 (8-bit) count interrupt cycle
// 	TIMSK must be OR'd to make sure to not mess with other timers.
// */

// void initTimer0(void){
// 	TCCR0 = (1 <<CS00) | (1<<CS02) | (1 << WGM01);
// 	TIMSK |= (1<<OCIE0);
// 	OCR0 = 255;

// }





