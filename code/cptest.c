#include "global.h"

#include "delay.h"

#include "eeprom.h"
#include "flash.h"
#include "uart.h"
#include "put.h"
#include "time.h"
#include "beep.h"

#include "dallas.h"
#include "led.h"



void main() {
    // LED
    led_init();
    led_flash();
    delay_s(1);

    // UART
    uart_init();
    uart_set_baud_rate(115200);
    led_flash();
    
    putProg("Welcome to Checkpoint Test!"); putCRLF();
    led_flash();

    putProg("TIME"); putCRLF();
    time_init();
    time_set(500UL);
    delay_s(2);
    // should register 1 or 2 seconds
    uint32_t time = time_get();
    putProg("Time test: time = "); putInt(time); putCRLF();
    if (time != 501 && time != 502) {
	// test failed
	putProg("Failed"); putCRLF();
	while(1);
    }
    led_flash();

    putProg("EEPROM"); putCRLF();
    uint8_t val = eeprom_read(0);
    val = ~val;
    eeprom_write(0, val);
    uint8_t read = eeprom_read(0);
    if (read != val) {
	putProg("Failed: written ");
	putHexF(val, 2);
	putProg(" read  ");
	putHexF(read, 2);
	putCRLF();
    }
    led_flash();

    putProg("BEEP"); putCRLF();
    beep_init();
    beep_yes();
    delay_ms(500);
    beep_no();
    delay_ms(500);
    led_flash();

    putProg("FLASH"); putCRLF();
    flash_init();
    flash_activate();
    for (int i = 0; i < 512; ++i) {
	uint8_t v = (i & 0xff);
	flash_buf_write(0, i, &v, 1);
    }
    led_flash();
    flash_put_buffer(0, 0);
    flash_get_buffer(1, 0);
    uint8_t ok = 1;
    for (int i = 0; i < 512; ++i) {
	uint8_t v;
	flash_buf_read(1, i, &v, 1);
	if (v != (i & 0xff)) {
	    putProg("Failed at ");
	    putHexF(i, 3);
	    putProg(", written ");
	    putHexF(i & 0xff, 2);
	    putProg(" read ");
	    putHexF(v, 2);
	    putCRLF();
	    ok = 0;
	}
    }
    led_flash();

    putProg("I-BUTTON"); putCRLF();
    dallasInit();
    led_on();
    uint8_t res;
    while(1) {
	res = dallasReset();
	if (res == DALLAS_PRESENCE) {
	    dallas_rom_id_T rom;
	    res = dallasReadROM(&rom);
	    if (res == DALLAS_NO_ERROR) {
		putProg("touched ");
		for(int8_t i = 7; i >= 0; i--)
		{
		    putHexF(rom.byte[i], 2);
		    putChar(' ');
		}
		putCRLF();
		led_off();
		break;
	    }
	}
    }
    delay_s(1);
    led_on();
    
    while(1);
}
