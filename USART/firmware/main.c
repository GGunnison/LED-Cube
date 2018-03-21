/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */


#include <avr/io.h>
// #include "USARTInit.h"
#include <avr/interrupt.h>
#include <util/delay.h>


#define bit_set(p,m) ((p) |= (m));
#define bit_flip(p,m) ((p) ^= (1<<m));
#define bit_clear(p,m) ((p) &= ~(m));

unsigned int BAUDRATE = 9600;
unsigned char newData = 0;
int one = 0x63;
int zero = 0x00;
int two = 0x44;



void uart_transmit(unsigned char data){

	while (!(UCSRA & (1<<UDRE))); 	// wait while register is free
	UDR = data;						// load data in the register
}

unsigned char uart_receive(){

	while(!((UCSRA) & (1<<RXC)));		// wait while data is being received
	return UDR;					// return 8-bit data
}

void USART_Init (unsigned int baud)
{
	//Put the upper part of the baud number here (bits 8 to 11)
	UBRRH = (unsigned char) (baud >> 8);

	//Put the remaining part of the baud number here
	UBRRL = (unsigned char) baud; 

	//Enable the receiver and transmitter
	UCSRB = (1 << RXEN) | (1 << TXEN);

	//Set 2 stop bits and data bit length is 8-bit
	UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);
}

int main(data)
{

	

	DDRA |= 0xFF;

 	USART_Init(103);
 	char ReceivedByte;
 	for (;;) // Loop forever 
   { 
      while ((UCSRA & (1 << RXC)) == 0) {}; // Do nothing until data have been recieved and is ready to be read from UDR 
      ReceivedByte = UDR; // Fetch the recieved byte value into the variable "ByteReceived" 
      while ((UCSRA & (1 << UDRE)) == 0) {}; // Do nothing until UDR is ready for more data to be written to it 
      UDR = ReceivedByte; // Echo back the received byte back to the computer 
   }
 
 }

