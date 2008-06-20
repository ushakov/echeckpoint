#include "global.h"

#include <avr/io.h>
#include "config.h"
#include "delay.h"

void led_init() {
    LED_DDR |= (1 << LED_PIN);
}

void led_on() {
    LED_PORT |= (1 << LED_PIN);
}

void led_off() {
    LED_PORT &= ~(1 << LED_PIN);
}

void led_flash () {
    led_on();
    delay_ms (500);
    led_off();
    delay_ms (500);
}

#ifdef LED_DIGITS
void led_flash_digit (int what) {
    if (what == 0) what = 10;
    for (int i = 0; i < what; ++i) {
	led_flash();
    }
    delay_ms (1000);
}

void led_flash_number (int number) {
    int highest = 1;
    while (highest * 10 <= number) highest *= 10;
    while (highest >= 1) {
	led_flash_digit (number / highest);
	number %= highest;
	highest /= 10;
    }
}
#endif // LED_DIGITS
