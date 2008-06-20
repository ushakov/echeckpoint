#include "global.h"

#include <avr/io.h>
#include "delay.h"
#include "config.h"

void beep_init() {
    BEEP_DDR |= (1 << BEEP_PIN);
    LED_DDR |= (1 << LED_PIN);
}

void beep_yes() {
    LED_PORT |= (1 << LED_PIN);
    for (int i = 0; i < 300; i++) {
	BEEP_PORT |= (1 << BEEP_PIN);
	delay_us(150);
	BEEP_PORT &= ~(1 << BEEP_PIN);
	delay_us(150);
    }
    LED_PORT &= ~(1 << LED_PIN);
    delay_ms(100);
}

void beep_no() {
//    LED_PORT |= (1 << LED_PIN);
    for (int i = 0; i < 1000; i++) {
	BEEP_PORT |= (1 << BEEP_PIN);
	delay_us(300);
	BEEP_PORT &= ~(1 << BEEP_PIN);
	delay_us(300);
    }
//    LED_PORT &= ~(1 << LED_PIN);
    delay_ms(100);
}
