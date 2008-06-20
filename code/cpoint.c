#include "global.h"

#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/io.h>

#include "delay.h"

#include "eeprom.h"
#include "flash.h"
#include "uart.h"
#include "put.h"
#include "time.h"
#include "beep.h"

#include "dallas.h"

#include "filesys.h"
#include "priority_queue.h"
#include "cp.h"

// if we've seen no button in 15 minutes, go to sleep
// when idle, we loop 10 times per second, so 15 min = 10 * 60 * 15 = 9000 loops
// #define MAX_IDLE_LOOPS 9000

// for debug, use a smaller value (10s)
#define MAX_IDLE_LOOPS 100

static int16_t get_num() {
    uint16_t n = 0;
    int16_t c;
    putCRLF(); putChar('#');
    do {
	while((c = uart_getchar()) == -1);
	if (c == '\r') return n;
	if (c >= '0' && c <= '9') {
	    n *= 10;
	    n += c - '0';
	} else {
	    putChar('?'); putCRLF();
	    n = 0;
	}
    } while (1);
}

static void monitor() {
    while(1) {
	putChar('>');
	int16_t c;
	while ((c = uart_getchar()) == -1) ;
	switch(c) {
	case 'd':
	    for (uint32_t addr = 0; addr < 0x1fffffUL; addr += 512) {
		flash_read(addr, buf, 512);
		for(uint16_t off = 0; off < 512; off++) {
		    if ((off & 0xf) == 0) {
			putCRLF();
			putHexF(addr + off, 8);
			putChar(' ');
		    }
		    putChar(' ');
		    putHexF(buf[off], 2);
		}
	    }
	    putCRLF();
            // FALLTHROUGH
	case 'i':
	    // Info
	    putChar('C');
	    putHexF(cpid, 2);
	    putCRLF();
	    putChar('M');
	    putHexF(mmbid, 4);
	    putCRLF();
	    putChar('B');
	    putHexF(current_no, 4);
	    putCRLF();
	    break;
	case 'c':
	    // Set cpid
	    cpid = get_num();
	    eeprom_write(EEP_CPID, cpid);
	    break;
	case 'm':
	    // Set mmbid
	    mmbid = get_num();
	    eeprom_write(EEP_MMBID_H, (mmbid >> 8) & 0xff);
	    eeprom_write(EEP_MMBID_L, mmbid & 0xff);
	    break;
	default:
	    putChar('?'); putCRLF(); beep_no();
	}
    }
}

void configure_all_pins_output() {
    DDRB = 0xff;
    DDRC = 0xff;
    DDRD = 0xff;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
}

void main() {
    configure_all_pins_output();
    set_sleep_mode(SLEEP_MODE_IDLE);
    dallasInit();
    time_init();
    flash_init();
    filesys_init();
    pq_init();
    cp_init();

    // look if we need a monitor
    uart_init();
    beep_init();
    uart_set_baud_rate(19200);
    delay_s(1);
    if (uart_getchar() != -1) {
	monitor();
    }
//    don't turn uart off in debug mode
//    uart_off();
    beep_yes();

    // Main loop. It must take lot less than a second
    // to make time accounting go smooth and iButton be responsive.
    uint8_t res;
    uint16_t idle_counter = 0;
    while(1) {
	res = dallasReset();
	if (res == DALLAS_PRESENCE) {
	    // We've got a button!
	    idle_counter = 0;
	    dallas_rom_id_T rom;
	    res = dallasReadROM(&rom);
	    if (res == DALLAS_NO_ERROR) res = dallasAddressCheck(&rom);
	    if (res == DALLAS_NO_ERROR) {
		switch (rom.byte[DALLAS_FAMILY_IDX]) {
		case 0x01:   // normal button
		case 0x0c:   // 8kb button
		    cp_register(&rom);
		    break;
		case 0x04:   // clock button
		{
		    uint32_t tm;
		    uint32_t prev_tm = 0;
		    uint16_t id;
		    uint16_t prev_id = 0;
		    for (int8_t i = 0; i < 6; ++i) {
			tm = dallasTimeRead();
			dallasReadRAM(0, 2, (uint8_t *) &id);
			if (tm == prev_tm && id == prev_id) {
			    time_set(tm);
			    filesys_reset();
			    cp_reset(id);
			    pq_reset();
			    beep_yes();
			    break;
			}
			prev_tm = tm;
			prev_id = id;
		    }
		    break;
		}
		default:
		    beep_no();
		}
	    }
	} else {
	    // we should not overload the counter
	    if (idle_counter < MAX_IDLE_LOOPS) {
		idle_counter++;
	    }
	}
	    
	flash_commit();
	time_check();

	// if we've been idle too long, go to sleep.
	if (idle_counter < MAX_IDLE_LOOPS) {
	    delay_ms(100);
	} else {
	    // go to sleep
	    dallasGoToSleep();
	    sei();
	    sleep_mode();
	    dallasOutOfSleep();
	}
    }
    
    while(1) { }
}

