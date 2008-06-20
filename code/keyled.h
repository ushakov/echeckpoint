#ifndef KEYLED_H
#define KEYLED_H

// Common part.

// Initialize.
void keyled_init();

// Activate display (taking pins from SPI)
void keyled_activate();

// Do one tick (light one digit, query one keyboard row).
// This is the only function that needs display activated before use.
// All other functions can be called with display deactivated.
void keyled_tick();

// Display part.

// Set display's digit area to display str, which is a <=4-char string
// describing the display.  Digits are encoded as-is, other chars to
// come later.
void keyled_set_display(char *str);

// Set display's digit area to display number n.
void keyled_set_display_num(uint32_t n);

// Set dots on display (m is a bitmask)
void keyled_set_dots (uint8_t m);


// Keyboard part.

// Any key pressed and not retrieved?
uint8_t keyled_keypressed();

// Retrieve the pressed key, reset "pressed" state
// returns: 0xff no key, 0-9 digits, 10 = *, 11 = #
uint8_t keyled_getkey();

#endif /* KEYLED_H */
