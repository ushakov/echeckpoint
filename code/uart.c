#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "delay.h"
#include "lcd.h"
#include "eeprom.h"

static int errors;

#define BUFLEN 16
#define BUFLENMASK 0x15
static uint8_t buffer[BUFLEN];
static uint8_t volatile bstart, blen;

ISR(USART_RXC_vect) {
    char data = UDR;
    uint8_t place = (bstart + blen) & BUFLENMASK;
    buffer[place] = data;
    blen++;
    sei();
}

void uart_init() {
    UCSRA = 0; //(1 << U2X);
    UCSRB = (1 << RXEN ) | (1 << TXEN) | (1 << RXCIE);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0); // | (1 << UPM1) | (1 << UPM0) 
    UBRRH = 0;
    UBRRL = 12;
    bstart = 0;
    blen = 0;
    errors = 0;
    PIND |= (1 << PIND0);
    sei();
}

void uart_off() {
    UCSRA = 0; //(1 << U2X);
    UCSRB = 0;
    UCSRC = 0;
    sei();
}

// set the uart baud rate
void uart_set_baud_rate(uint32_t baudrate)
{
    // calculate division factor for requested baud rate, and set it
    uint16_t bauddiv = ((F_CPU+(baudrate*8L))/(baudrate*16L)-1);
    outb(UBRRL, bauddiv);
#ifdef UBRRH
    outb(UBRRH, bauddiv>>8);
#endif
}

void uart_write_byte (uint8_t data) {
    while ( !(UCSRA & (1 << UDRE)) ) ;
    UDR = data;
}

int16_t uart_getchar()
{
    if (blen > 0) {
	char c = buffer[bstart];
	cli();
	bstart = (bstart + 1) & BUFLENMASK;
	blen--;
	sei();
	return c;
    }
    return -1;
}

