#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "dallas.h"
#include "vt100.h"
#include "put.h"
#include "delay.h"
#include "time.h"

#include "keyled.h"
#include "flash.h"

#include "beep.h"
#include "led.h"
#include "term.h"
#include "eeprom.h"

#include "data.h"


#include "team_numbers.h"
#include "time_set.h"

#include "apps.h"
uint8_t current_app;
uint16_t skip_counter;
uint8_t got_button;

void main() {
    beep_init();
    term_init();
    dallasInit();
    led_init();
    flash_init();
    time_init();
    
    uart_init();
    uart_set_baud_rate(115200);

    current_app = APP_TEAM_NUMBERS;

    got_button = 0;
    skip_counter = 0;
    team_numbers_init();
    timeset_init();
    term_init();
    while(1) {
	time_check();
	keyled_set_dots(1 << current_app);
	term_tick();
	if (skip_counter > 0) skip_counter--;
	got_button = 0;
	if (skip_counter == 0 && dallasReset() == DALLAS_PRESENCE) {
	    got_button = 1;
	    skip_counter = 500;
	}

	// app switching
	if (term_got_command()) {
	    uint32_t command = term_value();
	    if (command == 8463) {  // time
		current_app = APP_TIME_SET;
		timeset_wakeup();
		continue;
	    } else if (command == 8326) {  // team
		current_app = APP_TEAM_NUMBERS;
		continue;
	    }
	}
	if (current_app == APP_TEAM_NUMBERS) {
	    team_numbers();
	} else if (current_app == APP_TIME_SET) {
	    timeset();
	}
	delay_ms(1);
    }
}
