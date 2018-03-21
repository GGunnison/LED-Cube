/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */
#ifndef UARTInit 
#define UARTInit 

#define F_CPU 16000000UL


#include <avr/io.h>
#include <math.h>

#define EVEN 0
#define ODD 1

void uart_init(int baud, char asyncDoubleSpeed, int dataLength, char parity, char stopBitNum)
{
	uint16_t UBBRValue = lrint(((F_CPU)/(baud*16UL))-1);

	// Put the upper part of the baud number here (bits 8 to 11)
	UBRRH = (unsigned char) (UBBRValue >> 8);

	//Put the lower part of the baud number here (bits 0 to 7)
	UBRRL = (unsigned char) UBBRValue;

	//Setting the U2X bit to 1 for double speed
	//Default is not double speed.
	if (asyncDoubleSpeed == 1) UCSRA = (1<<U2X); 

	//Set 2 stop bits 
	if(stopBitNum) UCSRC = (1 << USBS);

	if (dataLength == 6) UCSRC |= (1 << UCSZ0); //6-bit data length
	if (dataLength == 7) UCSRC |= (2 << UCSZ0); //7-bit data length
	if (dataLength == 8) UCSRC |= (3 << UCSZ0); //8-bit data length
	if (dataLength == 9) UCSRC |= (7 << UCSZ0); //9-bit data length


	if (!parity) UCSRC |= (1 << UPM1); //Sets parity to EVEN
	if (parity) UCSRC |= (3 << UPM0);   //Sets parity to ODD
		

	//Enable receiver & transmitter
	UCSRB = (1<<RXEN) | (1<< TXEN);

	// Set frame format: 8 data 2 stop bit.
	UCSRC = (1<<URSEL);
    
}

void uart_transmit(unsigned char data){

	while (!(UCSRA & (1<<UDRE))); 	// wait while register is free
	UDR = data;						// load data in the register
}

unsigned char uart_receive(){

	while(!((UCSRA) & (1<<RXC)));		// wait while data is being received
		return UDR;					// return 8-bit data

}

#endif