#include "time_set.h"

#include "dallas.h"
#include "vt100.h"
#include "put.h"
#include "delay.h"

#include "keyled.h"
#include "flash.h"

#include "beep.h"
#include "led.h"
#include "term.h"
#include "eeprom.h"
#include "time.h"

#include "data.h"
#include "apps.h"


#define ST_DAY 1
#define ST_TIME 2
#define ST_DONE 3
#define ST_NUMBER 4
static uint8_t state;

uint8_t day, hour, min;
uint8_t mmb;

void timeset_init() {
    state = ST_DAY;
    day = 0;
    hour = 0;
    min = 0;
}

void timeset_wakeup() {
    state = ST_DAY;
    term_message("dAY");
}

void timeset() {
    if (term_got_reset()) {
	return;
    } else if (term_got_number()) {
	if (state == ST_DAY) {
	    day = term_value();
	    state = ST_TIME;
	    term_message("CL0C");
	} else if (state == ST_TIME) {
	    uint32_t t = term_value();
	    hour = t / 100;
	    min = t % 100;
	    state = ST_NUMBER;
	    time_set(86400UL * day + 3600UL * hour + 60UL * min);
	    term_message("n0");
	} else if (state == ST_NUMBER) {
	    mmb = term_value();
	    state = ST_DONE;
	    term_message("d0nE");
	}
    } else if (term_got_command()) {
	term_message("Err");
    }
    if (got_button) {
	dallas_rom_id_T rom;
	if (dallasReadROM(&rom) != DALLAS_NO_ERROR) {
	    return;
	}
	if (dallasAddressCheck(&rom) != DALLAS_NO_ERROR) {
	    return;
	}
	if (rom.byte[DALLAS_FAMILY_IDX] == 0x04) { // Clock button
	    uint32_t time = dallasTimeRead();
	    putProg("Time: ");
	    putHexF(time, 8);
	    putCRLF();

	    uint8_t e = DALLAS_NO_ERROR;
	    if (e == DALLAS_NO_ERROR) e = dallasTimeSet(time_get());
	    if (e == DALLAS_NO_ERROR) e = dallasWriteRAM(0, 1, &mmb);
	    
	    if (e == DALLAS_NO_ERROR) {
		term_message("600d");
		beep_yes();
		current_app = APP_TEAM_NUMBERS;
		return;
	    } else {
		term_message("bAd");
		putChar(e);
		putCRLF();
		return;
	    }
	}
    }
}

