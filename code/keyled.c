#include <inttypes.h>
#include <avr/io.h>

#include "config.h"
#include "delay.h"
#include "keyled.h"

uint8_t current_tick;
uint8_t display[4];  // videomem :)
                     // 0..3 = left-to-right

// *  +--+    0x4     0x8
//    |  |        0x2     0x10
//    +--+            0x1
//    |  |        0x80    0x40
//    +--+            0x20


uint8_t digitforms[] = {
    0xfa, 0x50, 0xb9,    // 0-2
    0x79, 0x53, 0x6b,    // 3-5
    0xeb, 0x58, 0xfb,    // 6-8
    0x7b                 // 9
};


uint8_t key_pressed = 0;
uint8_t key;

// Common part.

// Initialize.
// Activate display (taking pins from SPI)
void keyled_activate() {
    // Disable SPI.
    SPCR = 0;
    // Set directions / values
    DSP_PORT |= (1 << DSP_ENABLE);
    DSP_DDR |= (1 << DSP_ENABLE) | (1 << DSP_DATA) | (1 << DSP_CLOCK);
}

void keyled_init() {
    // Common wires, 4x
    SCAN_DDR |= (0xf << SCAN_SHIFT);
    SCAN_PORT |= (0xf << SCAN_SHIFT);

    // Keyboard inputs
    KEY_DDR &= ~(0x7 << KEY_SHIFT);
    // We need pull-ups here
    KEY_PORT |= (0x7 << KEY_SHIFT);
    
    current_tick = 0;
    for (int i = 0; i < 4; ++i) {
	display[i] = 0;
    }
}

uint8_t decode_key(uint8_t current_tick, uint8_t m) {

    uint8_t c = 0xff;
    if (m == 0x3) c = 0;
    if (m == 0x5) c = 1;
    if (m == 0x6) c = 2;
    if (c == 0xff) return 0xff;
    uint8_t r = 0xff;
    if (current_tick == 0) r = 3;
    if (current_tick == 1) r = 0;
    if (current_tick == 2) r = 1;
    if (current_tick == 3) r = 2;
    if (r == 0xff) return 0xff;

    uint8_t k = 1 + r*3 + c;
    if (k == 10) k = 10;
    if (k == 11) k = 0;
    if (k == 12) k = 11;
    return k;
}

// Do one tick (light one digit, query one keyboard row).
void keyled_tick() {
    current_tick = (current_tick + 1) % 4;
    // Turn off display/keys
    SCAN_PORT |= (0xf << SCAN_SHIFT);

    // Shift one digit to the display buffer
    uint8_t d = display[current_tick];
    for (uint8_t i = 0; i < 8; ++i) {
	// bring the clock low
	DSP_PORT &= ~(1 << DSP_CLOCK);
	// prepare the data
	DSP_PORT &= ~(1 << DSP_DATA);
	if (d & 0x1) {
	    DSP_PORT |= (1 << DSP_DATA);
	}
	// bring the clock up
	DSP_PORT |= (1 << DSP_CLOCK);
	// go to next bit
	d >>= 1;
    }
    
    // Set the scan pin to low
    uint8_t mask = (1 << current_tick) << SCAN_SHIFT;
    SCAN_PORT &= ~mask;
    delay_us(1);

    // Query keyboard
    uint8_t m = (KEY_PORTIN >> KEY_SHIFT) & 0x7;
    if (m != 0x7) {
	key_pressed = 1;
	key = decode_key(current_tick, m);
	while (((KEY_PORTIN >> KEY_SHIFT) & 0x7) != 0x7);
	delay_ms(20);
    }
}

// Display part.

// Set display's digit area to display str, which is a <=4-char string
// describing the display.  Digits are encoded as-is, other chars to
// come later.
void keyled_set_display(char *str) {
    for (uint8_t i = 0; i < 4; ++i) {
	display[i] = 0;
    }
    for (uint8_t i = 0; (i < 4) && str[i]; ++i) {
	if (str[i] >= '0' && str[i] <='9') {
	    display[i] = digitforms[str[i] - '0'];
	} else if (str[i] == '-') {
	    display[i] = 0x1;
	} else if (str[i] == '_') {
	    display[i] = 0x20;
	} else if (str[i] == 'E') {
	    display[i] = 0xab;
	} else if (str[i] == 'r') {
	    display[i] = 0x81;
	} else if (str[i] == 'd') {
	    display[i] = 0xf1;
	} else if (str[i] == 'u') {
	    display[i] = 0xe0;
	} else if (str[i] == 'p') {
	    display[i] = 0x9b;
	} else if (str[i] == 'b') {
	    display[i] = 0xe3;
	} else if (str[i] == 't') {
	    display[i] = 0xa3;
	} else if (str[i] == 'n') {
	    display[i] = 0xc1;
	} else if (str[i] == 'A') {
	    display[i] = 0xdb;
	} else if (str[i] == 'Y') {
	    display[i] = 0x73;
	} else if (str[i] == 'h') {
	    display[i] = 0xc3;
	} else if (str[i] == 'C') {
	    display[i] = 0xaa;
	} else if (str[i] == 'L') {
	    display[i] = 0xa2;
	} else if (str[i] == ' ') {
	    display[i] = 0x0;
	}
    }
}

// Set display's digit area to display number n.
void keyled_set_display_num(uint32_t n) {
    if (n == 0) {
	keyled_set_display("   0");
	return;
    }

    char s[5];
    int8_t i = 3;
    while (i >= 0) {
	if (n == 0) {
	    s[i] = ' ';
	} else {
	    s[i] = '0' + n % 10;
	}
	n /= 10;
	i--;
    }
    s[i] = 0;
    keyled_set_display(s);
}

// Set dots on display (m is a bitmask)
void keyled_set_dots (uint8_t m) {
    for (uint8_t i = 0; i < 4; ++i) {
	display[i] &= ~0x4; // assuming this is the dot
	if (m & (1 << i)) {
	    display[i] |= 0x4;
	}
    }
}

// Keyboard part.

// Any key pressed and not retrieved?
uint8_t keyled_keypressed() {
    return key_pressed;
}

// Retrieve the pressed key, reset "pressed" state
// returns: -1 no key, 0-9 digits, 10 = *, 11 = #
uint8_t keyled_getkey() {
    if (!key_pressed) {
	return -1;
    } else {
	key_pressed = 0;
	return key;
    }
}
