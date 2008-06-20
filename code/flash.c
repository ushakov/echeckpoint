#include "global.h"
#include "config.h"

#include <inttypes.h>
#include <avr/io.h>
#include "delay.h"
#include "beep.h"

#include "flash.h"

static uint8_t flash_transmit(uint8_t b);
static uint8_t flash_wait();

// read/write mem to buffer
static void flash_get_buffer(uint8_t buffer, uint16_t page);
static void flash_put_buffer(uint8_t buffer, uint16_t page);

// read/update buffers
static void flash_buf_read(uint8_t buffer, uint16_t off, uint8_t *mem, uint16_t len);
static void flash_buf_write(uint8_t buffer, uint16_t off, uint8_t *mem, uint16_t len);

// read directly
static void flash_read_directly(uint16_t page, uint16_t off, uint8_t *mem, uint16_t len);

#define SPI_START     SPI_PORT &= ~(1 << SPI_SS)
#define SPI_END     SPI_PORT |= (1 << SPI_SS)

int16_t page_in_buffer;
int16_t autorewrite_counter;

void flash_init() {
    page_in_buffer = -1;

    // check if flash is in 512-byte mode,
    // and if not, switch it over and flash LED
    flash_activate();
    uint8_t status = flash_wait();
    if ((status & 0x1) == 0) {
	// flash is set to 528 bytes/sector.
	// Switch it over to 512 bytes/sector
	// and flash LED/beep since this requires a powercycle.
	SPI_START;
	flash_transmit(0x3d);
	flash_transmit(0x2a);
	flash_transmit(0x80);
	flash_transmit(0xa6);
	SPI_END;
	while (1) {
	    beep_yes();
	}
    }
}

void flash_activate() {
    SPI_PORT |= (1 << SPI_SS);
    SPI_DDR |= (1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS);
    SPI_DDR &= ~(1 << SPI_MISO);
    
    /* Enable SPI, Master, set clock rate f/2 */
    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR = (1 << SPI2X);

    // Set idle SCK to low
    SPI_PORT &= ~(1 << SPI_SCK);
}

static uint8_t flash_transmit(uint8_t b)
{
    SPDR = b;
    while(!(SPSR & (1 << SPIF)));
    return SPDR;
}

static uint8_t flash_wait() {
    uint8_t c;
    SPI_START;
    flash_transmit(0xd7);
    do {
	c = flash_transmit(0);
    } while((c & 0x80) == 0);
    SPI_END;
    return c;
}

#if 0
uint32_t flash_id() {
    uint8_t c = 0;
    uint32_t n = 0;
    SPI_START;
    flash_transmit(0x9f);
    c = flash_transmit(0);
    n |= ((uint32_t)c) << 24;
    c = flash_transmit(0);
    n |= ((uint32_t)c) << 16;
    c = flash_transmit(0);
    n |= ((uint32_t)c) << 8;
    c = flash_transmit(0);
    n |= ((uint32_t)c) << 0;
    SPI_END;
    return n;
}
#endif

union address {
    uint32_t i;
    uint8_t b[4];
};

static void op_page (uint8_t op, uint16_t page) {
    flash_wait();

    union address addr;
    addr.i = page;
    addr.i <<= 9;
    
    SPI_START;
    flash_transmit(op);
    flash_transmit(addr.b[2]);
    flash_transmit(addr.b[1]);
    flash_transmit(addr.b[0]);
    SPI_END;
}

void flash_get_buffer(uint8_t buffer, uint16_t page) {
    if (buffer == 0) {
	op_page(0x53, page);
    } else {
	op_page(0x55, page);
    }
}

void flash_put_buffer(uint8_t buffer, uint16_t page) {
    if (buffer == 0) {
	op_page(0x83, page);
    } else {
	op_page(0x86, page);
    }
}

#if 0
// read/update buffers
void flash_buf_read(uint8_t buffer, uint16_t off, uint8_t *mem, uint16_t len) {
    flash_wait();

    SPI_START;
    if (buffer == 0) {
	flash_transmit(0xd1);
    } else {
	flash_transmit(0xd3);
    }
    flash_transmit(0);
    flash_transmit((off & 0xff00) >> 8);
    flash_transmit(off & 0xff);
    for (int i = 0; i < len; ++i) {
	*mem = flash_transmit(0);
	mem ++;
    }
    SPI_END;
}

#endif

void flash_buf_write(uint8_t buffer, uint16_t off, uint8_t *mem, uint16_t len) {
    flash_wait();

    SPI_START;
    if (buffer == 0) {
	flash_transmit(0x84);
    } else {
	flash_transmit(0x87);
    }
    flash_transmit(0);
    flash_transmit((off & 0xff00) >> 8);
    flash_transmit(off & 0xff);
    for (int i = 0; i < len; ++i) {
	flash_transmit(*mem);
	mem ++;
    }
    SPI_END;
}

// read directly
void flash_read_directly(uint16_t page, uint16_t off, uint8_t *mem, uint16_t len){
    flash_wait();

    union address addr;
    addr.i = page;
    addr.i <<= 9;
    addr.i += off;

    SPI_START;
    flash_transmit(0x03);
    flash_transmit(addr.b[2]);
    flash_transmit(addr.b[1]);
    flash_transmit(addr.b[0]);
    for (int i = 0; i < len; ++i) {
	*mem = flash_transmit(0);
	mem ++;
    }
    SPI_END;
}

static void flash_autorewrite_something() {
    // need to autorewrite the page number
    // autorewrite_counter
    op_page(0x58, autorewrite_counter);
    autorewrite_counter += 1;
    autorewrite_counter &= 0xfff;   // page count
}


void flash_read(uint32_t addr, uint8_t *mem, uint16_t len) {
    uint16_t page = addr >> 9;
    uint16_t off = addr & 0x1ff;
    if (page == page_in_buffer) {
	flash_commit();
    }
    flash_read_directly(page, off, mem, len);
}

static void flash_write_in_page(uint16_t page, uint16_t off, uint8_t *mem, uint16_t len) {
    if (page != page_in_buffer) {
	if (page_in_buffer != -1) {
	    flash_put_buffer(0, page_in_buffer);
	    flash_autorewrite_something();
	}
	flash_get_buffer(0, page);
	page_in_buffer = page;
    }
    flash_buf_write(0, off, mem, len);
}

void flash_commit() {
    if (page_in_buffer != -1) {
	flash_put_buffer(0, page_in_buffer);
	flash_autorewrite_something();
	page_in_buffer = -1;
    }
}

void flash_write(uint32_t addr, uint8_t *mem, uint16_t len) {
    uint16_t page = addr >> 9;
    uint16_t off = addr & 0x1ff;
    while (len > 0) {
	uint16_t to_write = 512 - off;
	if (to_write > len) to_write = len;
	flash_write_in_page(page, off, mem, to_write);
	mem += to_write;
	len -= to_write;
	off = 0;
	page++;
    }
}
