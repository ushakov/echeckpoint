#include "global.h"
#include <inttypes.h>

#include "uart.h"
#include "put.h"
#include "dallas.h"

static int16_t get_hex() {
    uint16_t n = 0;
    int16_t c;
    for(uint8_t i = 0; i < 2; i++) {
	while((c = uart_getchar()) == -1);
	n <<= 4;
	if (c >= '0' && c <= '9') {
	    n += c - '0';
	} else {
	    n += c - 'a' + 10;
	}
    }
    return n;
}


// dallasInit()
//     Initializes the Dallas 1-wire Bus
//     Currently this function does nothing
void dallasInit(void) {
}

void dallasGoToSleep(void)
{
    uart_write_byte('S');
    uart_write_byte('\n');
    uart_write_byte('\r');
}

void dallasOutOfSleep(void)
{
    uart_write_byte('U');
    uart_write_byte('\n');
    uart_write_byte('\r');
}

// dallasReset()
//     performs a reset on the 1-wire bus
//     returns the presence detect (DALLAS_PRESENCE or DALLAS_NO_PRESENCE)
u08  dallasReset(void) {
    int16_t c = uart_getchar();
    if (c == -1) return DALLAS_NO_PRESENCE;
    return DALLAS_PRESENCE;
}

// dallasReadRAM()
//     reads the RAM from the specified device, at the specified RAM address
//     for the specified length.  Data is stored into data variable
u08  dallasReadRAM(u16 addr, u08 len, u08 *data) {
    uart_write_byte('R');
    putHexF(addr, 4);
    putHexF(len, 2);
    for (uint8_t i = 0; i < len; ++i) {
	data[i] = get_hex();
    }
    uart_write_byte('X');
    uart_write_byte('\n');
    return DALLAS_NO_ERROR;
}

// dallasWriteRAM()
//     writes the specified data for the specified length to the RAM
//     located at the specified address of the specified device
u08  dallasWriteRAM(u16 addr, u08 len, u08* data) {
    uart_write_byte('W');
    putHexF(addr, 4);
    putHexF(len, 2);
    for (uint8_t i = 0; i < len; ++i) {
	putHexF(data[i], 2);
    }
    uart_write_byte('X');
    uart_write_byte('\n');
    return DALLAS_NO_ERROR;
}

// dallasReadROM()
//     finds the ROM code of a device if only 1 device is
//     connected to the bus the ROM value is passed by referenced
//     returns any error that occured or DALLAS_NO_ERROR
u08 dallasReadROM(dallas_rom_id_T* rom_id) {
    uart_write_byte('I');
    for (int i = 0; i < 8; ++i) {
	rom_id->byte[i] = get_hex();
    }
    uart_write_byte('X');
    for (int i = 0; i < 8; ++i) {
	putHexF(rom_id->byte[i], 2);
    }
    uart_write_byte('\n');
    return DALLAS_NO_ERROR;
}

// dallasAddressCheck()
//     checks to make sure that the crc of the id is correct
//     returns the corresponding error or DALLAS_NO_ERROR
u08  dallasAddressCheck(dallas_rom_id_T* rom_id) {
    if (rom_id->byte[DALLAS_CRC_IDX] != 0xff) {
	uart_write_byte('X');
	putHexF(rom_id->byte[DALLAS_CRC_IDX], 2);
	uart_write_byte('\n');
	return DALLAS_CRC_ERROR;
    } else {
	return DALLAS_NO_ERROR;
    }
}

// time functions
uint32_t dallasTimeRead() {
    uint32_t d = 0;
    dallasReadRAM(0xffff, 4, (uint8_t *) &d);
    return d;
}

uint8_t dallasTimeSet(uint32_t time) {
    dallasWriteRAM(0xffff, 4, (uint8_t *) &time);
    return DALLAS_NO_ERROR;
}
