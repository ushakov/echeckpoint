#include "term.h"

#include "keyled.h"
#include "flash.h"
#include "uart.h"
#include "put.h"

static uint32_t n;

// 0 -- normal
// 1 -- just after reset (to catch second reset)
// 2 -- command mode
// 3 -- special signal (implicitly resets on first key)
static uint8_t state = 0;

// 0 -- normal; entering digits
// 1 -- got command
// 2 -- got number
// 3 -- need to clear n
// 0x80 (or'ed) -- just been reset
static uint8_t flags;

void term_init() {
    keyled_init();
    n = 0;
    state = 0;
    keyled_set_display_num(n);
}

void term_tick() {
    if (flags != 0) {
	n = 0;
	keyled_set_display_num(n);
    }
    flags = 0;
    if (state == 2) {
	keyled_set_dots(0xff);
    }
    
    keyled_activate();
    keyled_tick();
    flash_activate();

    if (keyled_keypressed()) {
	int k = keyled_getkey();

	if (state == 3) {  // return to normal after message
	    state = 0;
	}
	
	if (k == 11) {  // Submit
	    if (state == 0 || state == 1) {
		flags = 2;
		state = 0;
	    } else if (state == 2) {
		flags = 1;
		state = 0;
	    }
	} else if (k == 10) { // Reset
	    if (state == 0) {
		state = 1;
	    } else if (state == 1) {
		state = 2;
	    } else if (state == 2) {
		state = 1;
	    }
	    flags = 3;
	} else {
	    n = n * 10;
	    n += k;
	    if (state == 1) {
		state = 0;
	    }
	    keyled_set_display_num(n);
	}
    }
    
}

// a command has been committed this tick
uint8_t term_got_command() {
    return flags == 1;
}

// a number has been committed this tick
uint8_t term_got_number() {
    return flags == 2;
}

// the terminal has been reset this tick
uint8_t term_got_reset() {
    return flags == 3;
}

// value of command/number enetered
uint32_t term_value() {
    return n;
}

void term_message(char *msg) {
    keyled_set_display (msg);
    state = 3;
    flags = 0;
    n = 0;
}
