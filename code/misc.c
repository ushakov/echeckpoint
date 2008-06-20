#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "misc.h"

void put_int (int number, char *c) {
    int highest = 1;
    while (highest * 10 <= number) highest *= 10;
    while (highest >= 1) {
	*c++ = '0' + number / highest;
	number %= highest;
	highest /= 10;
    }
    *c = '\0';
}
