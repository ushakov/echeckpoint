#ifndef TERM_H
#define TERM_H

#include <inttypes.h>

void term_init();
void term_tick();

// a command has been committed this tick
uint8_t term_got_command();

// a number has been committed this tick
uint8_t term_got_number();

// the terminal has been reset this tick
uint8_t term_got_reset();

// value of command/number enetered
uint32_t term_value();

// Display text message
void term_message(char *msg);

#endif /* TERM_H */
