#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "dallas.h"
#include "vt100.h"
#include "put.h"
#include "delay.h"

#include "keyled.h"
#include "flash.h"

#include "beep.h"
#include "led.h"
#include "term.h"

dallas_rom_id_T last[10];
u08 count = 0;

extern uint8_t current_tick;
extern uint8_t display[4];

#include "config.h"

uint16_t off = 0;
uint16_t page = 0;

void commit_page() {
    flash_put_buffer(0, page);
    page ++;
    off = 0;
}

void commit(uint16_t n) {
    if (off == 0) {
	putProg("starting page ");
	putInt(page);
	putCRLF();
    }

    flash_buf_write(0, off, (uint8_t *) &n, 2);
    off += 2;

    if (off == 200) {
	// need to write page and start a new one
	commit_page();
    }
}

void dump_pages() {
    if (off != 0) {
	commit_page();
    }
    
    int t = 0;
    for (int p = 0; p < page; ++p) {
	for (int n = 0; n < 200; ++n) {
	    uint16_t num;
	    flash_read_directly(p, n*2, (uint8_t *) &num, 2);
	    putHexF(num, 4);
	    if (t == 16) {
		putCRLF();
		t = 0;
	    } else {
		putProg(" ");
		t++;
	    }
	}
    }
}

int main () {
    uart_init();
    uart_set_baud_rate(115200);
    vt100Init();
    dallasInit();
    flash_init();
    
    vt100ClearScreen();
    vt100SetAttr(VT100_BOLD);
    putProg("Welcome to MMB Terminal!"); putCRLF();
    vt100SetAttr(VT100_ATTR_OFF);

    flash_activate();
    putProg("Flash status: ");
    uint8_t c = flash_wait();
    putHex(c); putCRLF();

    putProg("Flash ID: ");
    uint32_t id = flash_id();
    putHex(id); putCRLF();

    term_init();
    while(1) {
	term_tick();
	
	delay_us(200);
	if (term_got_command()) {
	    uint32_t n = term_value();
	    // do command processing here
	    if (n == 3867) { // DUMP
		putProg("DUMP"); putCRLF();
		dump_pages();
	    } else if (n == 35874) { // FLUSH
		putProg("FLUSH"); putCRLF();
		commit_page();
	    } else {
		putProg("ERR"); putCRLF();
		term_message("Err");
	    }
	} else if (term_got_number()) {
	    uint32_t n = term_value();
	    putProg("Commit! n = ");
	    putInt(n);
	    putCRLF();
	    commit(n);
	}
    }
    return 0;
}
