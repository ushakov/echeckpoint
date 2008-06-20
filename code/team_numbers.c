#include "team_numbers.h"

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

team_info info;
uint8_t have_button;

int16_t pages_initialized;
int16_t current_page;

void flush_block() {
    flash_put_buffer(0, current_page);
    current_page = -1;
}

void pick_block(int16_t page) {
    if (page == current_page) {
	return;
    }
    if (current_page != -1) {
	flush_block();
    }
    if (page >= pages_initialized) {
	uint8_t buf[16];
	for (int16_t i = 0; i < 16; ++i) {
	    buf[i] = 0xff;
	}
	for (int16_t i = 0; i < 512; i += 16) {
	    flash_buf_write(0, i, buf, 16);
	}
	for ( ; pages_initialized <= page; ) {
	    flash_put_buffer(0, pages_initialized);
	    pages_initialized++;
	}
	eeprom_write(EEP_MAXPAGE_L, (uint8_t)(pages_initialized & 0xff));
	eeprom_write(EEP_MAXPAGE_H, (uint8_t)(pages_initialized >> 8));
	putProg("Pages initialized: ");
	putInt(pages_initialized);
	putCRLF();
    }
    flash_get_buffer(0, page);
    current_page = page;
}

uint8_t have_number(uint16_t team_no) {
    uint16_t page = team_no / 46;   // 46 teams per 512-byte page
    if (page >= pages_initialized) {
	return 0;
    }
    int16_t off = (team_no % 46) * 11;
    uint16_t num;
    flash_read_directly(page, off, (uint8_t *) &num, 2);
    if (num == 0xffff) {
	return 0;
    }
    return 1;
}

// puts team_info
void put_team() {
    int16_t page = info.id / 46;   // 46 teams per 512-byte page
    pick_block(page);
    int16_t off = (info.id % 46) * 11;
    flash_buf_write(0, off, (uint8_t *) &info, 11);
    flush_block();
}

void dump_info() {
    team_info ti;
    for (int16_t page = 0; page < pages_initialized; ++page) {
	for (int16_t n = 0; n < 46; n++) {
	    flash_read_directly(page, n * 11, (uint8_t *) &ti, 11);
	    if (ti.id != 0xffff) {
		putHexF(ti.id, 4);
		putProg(" ");
		for (int8_t i = 0; i < 6; ++i) {
		    putHexF(ti.button[i], 2);
		}
		putProg(" [");
		putInt(ti.ts.dayhour >> 5);
		putProg("] ");
		putInt(ti.ts.dayhour & 0x1f);
		putProg(":");
		putInt(ti.ts.min);
		putProg(":");
		putInt(ti.ts.sec);
		putCRLF();
	    }
	}
    }
}


void team_numbers_init() {
    have_button = 0;
    current_page = -1;
    pages_initialized = 0;
    pages_initialized = eeprom_read(EEP_MAXPAGE_L);
    pages_initialized |= eeprom_read(EEP_MAXPAGE_H) << 8;
}

void team_numbers() {
    if (term_got_reset()) {
	have_button = 0;
	led_off();
    } else if (term_got_number()) {
	info.id = term_value();
	if (!have_button) {
	    term_message("butn");
	    return;
	}
	if (have_number(info.id)) {
	    term_message("dup");
	    return;
	}
	info.ts = time_get_ts();
	put_team();
	have_button = 0;
	led_off();
	beep_yes();
	beep_yes();
    } else if (term_got_command()) {
	uint32_t command = term_value();
	if (command == 3867) { // dump
	    dump_info();
	} else if (command == 73738) { // reset
	    pages_initialized = 0;
	    eeprom_write(EEP_MAXPAGE_L, 0);
	    eeprom_write(EEP_MAXPAGE_H, 0);
	    term_message("r5Et");
	} else {
	    term_message("Err");
	}
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
	    time_set(time);
	    term_message("600d");
	} else {
	    for (int8_t i = 0; i < 6; ++i) {
		info.button[i] = rom.byte[i+1];
	    }
	    led_on();
	    beep_yes();
	    have_button = 1;
	}
    }
}
