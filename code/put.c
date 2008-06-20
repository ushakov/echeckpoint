#include "global.h"
#include <avr/pgmspace.h>
#include <inttypes.h>
#include "uart.h"
#include "put.h"

void putProgR(const char* str) {
    register char c;
    while((c = pgm_read_byte(str++))) {
	uart_write_byte(c);
    }
}

void putCRLF() {
    putProg("\r\n");
}

#if 0
void putInt(uint32_t number) {
    if (number == 0) {
	uart_write_byte('0');
	return;
    }
    register uint32_t highest = 1;
    while (highest * 10 <= number) highest *= 10;
    while (highest >= 1) {
        uart_write_byte('0' + number / highest);
        number %= highest;
        highest /= 10;
    }
}

void putHex(uint32_t number) {
    if (number == 0) {
	uart_write_byte('0');
	return;
    }
    uint32_t highest = 1;
    while (highest < 0x10000000ULL && highest * 16 <= number) {
	highest *= 16;
    }
    while (highest >= 1) {
	register uint32_t t = number / highest;
	if (t > 9) {
	    uart_write_byte('a' + t - 10);
	} else {
	    uart_write_byte('0' + t);
	}
        number %= highest;
        highest /= 16;
    }
}

void putRevHex(uint32_t number) {
    while (number > 0) {
	register uint8_t t = number % 16;
	if (t > 9) {
	    uart_write_byte('a' + t - 10);
	} else {
	    uart_write_byte('0' + t);
	}
        number /= 16;
    }
}

void putRam(const char* str) {
    register char c;
    while((c = *str++)) {
	uart_write_byte(c);
    }
}
#endif

void putHexF(uint32_t number, uint8_t field) {
    register uint32_t highest = 1;
    register uint8_t len = 1;
    while ((highest < 0x10000000UL && highest * 16 <= number) || len < field) {
	highest *= 16;
	len++;
    }
    while (highest >= 1) {
	register uint32_t t = number / highest;
	if (t > 9) {
	    uart_write_byte('a' + t - 10);
	} else {
	    uart_write_byte('0' + t);
	}
        number %= highest;
        highest /= 16;
    }
}

void putChar(char c) {
    uart_write_byte(c);
}
