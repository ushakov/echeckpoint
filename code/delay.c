#include "global.h"

#include <util/delay.h>

#include "delay.h"

void delay_ms (int ms) {
    uint32_t cycles_to_do = (uint32_t)  ((F_CPU) / 4000 * ms);
    while (cycles_to_do > 60000) {
	_delay_loop_2 (60000);
	cycles_to_do -= 60000;
    }
    _delay_loop_2 ((uint16_t) cycles_to_do);
}

void delay_s (int s) {
    for (int i = 0; i < s; ++i) {
	delay_ms (1000);
    }
}

void delay_us(uint16_t us)
{
    uint32_t loops = (F_CPU / 1000) * us / 3000L;
    while (loops > 250) {
	_delay_loop_1 (250);
	loops -= 250;
    }
    _delay_loop_1 ((uint8_t) loops);
}
