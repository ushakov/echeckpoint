//*****************************************************************************
// File Name	: dallas.c
// Title		: Dallas 1-Wire Library
// Revision		: 6
// Notes		: 
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
// 
//*****************************************************************************
 
//----- Include Files ---------------------------------------------------------
#include <avr/io.h>				// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>		// include interrupt support
#include <string.h>				// include string support
#include "dallas.h"				// include dallas support
#include "put.h"
#include "delay.h"

#ifdef TABLE_CRC
//----- Global Variables -------------------------------------------------------
u08 dallas_crc;					// current crc global variable
u08 dallas_crc_table[] =		// dallas crc lookup table
{
	0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
	157,195, 33,127,252,162, 64, 30, 95, 1,227,189, 62, 96,130,220,
	35,125,159,193, 66, 28,254,160,225,191, 93, 3,128,222, 60, 98,
	190,224, 2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
	70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89, 7,
	219,133,103, 57,186,228, 6, 88, 25, 71,165,251,120, 38,196,154,
	101, 59,217,135, 4, 90,184,230,167,249, 27, 69,198,152,122, 36,
	248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91, 5,231,185,
	140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
	17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
	175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
	50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
	202,148,118, 40,171,245, 23, 73, 8, 86,180,234,105, 55,213,139,
	87, 9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
	233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
	116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};
#endif // TABLE_CRC

//----- Functions --------------------------------------------------------------

void dallasDelayUs(u16 us)
{
    u16 i;
    // one loop takes 8 cpu cycles (i.e. about 1us)
    for (i=0; i < us; i++) { __asm__ ( "nop" ); }
}

void dallasInit(void)
{
	// set line high
	cbi(DALLAS_DDR, DALLAS_PIN);
	sbi(DALLAS_PORT, DALLAS_PIN);
}

void dallasGoToSleep(void)
{
    sbi(DALLAS_DDR, DALLAS_PIN);
    cbi(DALLAS_PORT, DALLAS_PIN);
}

void dallasOutOfSleep(void)
{
    dallasInit();
    delay_ms(50);
}

u08 dallasReset(void)
{
	u08 presence = DALLAS_PRESENCE;

	cli();
	
	// pull line low
	sbi(DALLAS_DDR, DALLAS_PIN);
	cbi(DALLAS_PORT, DALLAS_PIN);
	
	// wait for presence
	dallasDelayUs(480);
	
	// allow line to return high
	cbi(DALLAS_DDR, DALLAS_PIN);
	sbi(DALLAS_PORT, DALLAS_PIN);
	
	// wait for presence
	dallasDelayUs(70);

	// if device is not present, pin will be 1
	if (inb(DALLAS_PORTIN) & (1<<DALLAS_PIN))
		presence = DALLAS_NO_PRESENCE;

	// wait for end of timeslot
	dallasDelayUs(410);

	sei();

	// now that we have reset, let's check bus health
	// it should be noted that a delay may be needed here for devices that
	// send out an alarming presence pulse signal after a reset
	//cbi(DALLAS_DDR, DALLAS_PIN);
	//sbi(DALLAS_PORT, DALLAS_PIN);
	//dallasDelayUs(200);
	if (!(inb(DALLAS_PORTIN) & (1<<DALLAS_PIN)))	// it should be pulled up to high
		return DALLAS_BUS_ERROR;

	return presence;
}

u08 dallasReadBit(void)
{
	u08 bit = 0;
	
	// pull line low to start timeslot
	sbi(DALLAS_DDR, DALLAS_PIN);
	cbi(DALLAS_PORT, DALLAS_PIN);
	
	// delay appropriate time
	dallasDelayUs(6);

	// release the bus
	cbi(DALLAS_DDR, DALLAS_PIN);
	sbi(DALLAS_PORT, DALLAS_PIN);
	
	// delay appropriate time	
	dallasDelayUs(9);

	// read the pin and set the variable to 1 if the pin is high
	if (inb(DALLAS_PORTIN) & 0x01<<DALLAS_PIN)
		bit = 1;
	
	// finish read timeslot
	dallasDelayUs(55);
	
	return bit;
}

void dallasWriteBit(u08 bit)
{
	// drive bus low
	sbi(DALLAS_DDR, DALLAS_PIN);
	cbi(DALLAS_PORT, DALLAS_PIN);
	
	// delay the proper time if we want to write a 0 or 1
	if (bit)
		dallasDelayUs(6);
	else
		dallasDelayUs(60);

	// release bus
	cbi(DALLAS_DDR, DALLAS_PIN);
	sbi(DALLAS_PORT, DALLAS_PIN);

	// delay the proper time if we want to write a 0 or 1
	if (bit)
		dallasDelayUs(64);
	else
		dallasDelayUs(10);
}

u08 dallasReadByte(void)
{
	u08 i;
	u08 byte = 0;

	cli();
	
	// read all 8 bits
	for(i=0;i<8;i++)
	{
		if (dallasReadBit())
			byte |= 0x01<<i;

		// allow a us delay between each read
		dallasDelayUs(1);
	}

	sei();

	return byte;
}

void dallasWriteByte(u08 byte)
{
	u08 i;

	cli();

	// write all 8 bits
	for(i=0;i<8;i++)
	{
		dallasWriteBit((byte>>i) & 0x01);
		
		// allow a us delay between each write
		dallasDelayUs(1);
	}
	
	sei();
}

u08 dallasReadRAM(u16 addr, u08 len, u08 *data)
{
	u08 error;
	error = dallasReset();
	if (error != DALLAS_PRESENCE) {
	    return error;
	}
	dallasWriteByte(DALLAS_SKIP_ROM);
	
	dallasWriteByte(0xf0);
	dallasWriteByte(addr & 0xff);
	dallasWriteByte(addr >> 8);
	for(uint8_t i=0;i<len;i++) {
	    data[i] = dallasReadByte();
	}
	
	error = dallasReset();
	if (error != DALLAS_PRESENCE) {
	    return error;
	}
	
	return DALLAS_NO_ERROR;
}

u08 dallasWriteRAMInPage(u16 addr, u08 len, u08* data)
{
	u08 error;
	error = dallasReset();
	if (error != DALLAS_PRESENCE) {
	    return error;
	}
	dallasWriteByte(DALLAS_SKIP_ROM);
	
	dallasWriteByte(0x0f);  // write scratchpad
	dallasWriteByte(addr & 0xff);
	dallasWriteByte(addr >> 8);
	for(uint8_t i=0; i < len; i++)
	{
	    dallasWriteByte(data[i]);
	}

	error = dallasReset();
	if (error != DALLAS_PRESENCE) {
	    return error;
	}
	dallasWriteByte(DALLAS_SKIP_ROM);
	
	dallasWriteByte(0xaa);  // read/check scratchpad
	if (dallasReadByte() != (addr & 0xff)) {
	    return DALLAS_VERIFY_ERROR;
	}
	if (dallasReadByte() != (addr >> 8)) {
	    return DALLAS_VERIFY_ERROR;
	}
	uint8_t es = dallasReadByte();
	for(uint8_t i=0; i < len; i++)
	{
	    if (dallasReadByte() != data[i]) {
		return DALLAS_VERIFY_ERROR;
	    }
	}

	error = dallasReset();
	if (error != DALLAS_PRESENCE) {
	    return error;
	}
	dallasWriteByte(DALLAS_SKIP_ROM);

	dallasWriteByte(0x55);  // copy scratchpad
	dallasWriteByte(addr & 0xff);
	dallasWriteByte(addr >> 8);
	dallasWriteByte(es);

	// wait until done
	uint8_t res;
	do {
	    cli();
	    res = dallasReadBit();
	    sei();
	} while (res != 0);

	return DALLAS_NO_ERROR;
}

u08 dallasWriteRAM(u16 addr, u08 len, u08* data) {
    while (len > 0) {
	u08 paddr = addr & 0x1f;
	u08 to_transmit = 32 - paddr;
	if (len < to_transmit) to_transmit = len;
	u08 res = dallasWriteRAMInPage(addr, to_transmit, data);
	if (res != DALLAS_NO_ERROR) {
	    return res;
	}
	len -= to_transmit;
	addr += to_transmit;
	data += to_transmit;
    }
    return DALLAS_NO_ERROR;
}

#ifdef TERMINAL
uint8_t dallasTimeSet(uint32_t time) {
    uint8_t error;
    // set time
    uint8_t bytes[5];
    bytes[0] = 0;
    bytes[1] = time & 0xff;
    bytes[2]= (time >> 8) & 0xff;
    bytes[3]= (time >> 16) & 0xff;
    bytes[4]= (time >> 24) & 0xff;
    error = dallasWriteRAM(0x202, 5, bytes);
    if (error != DALLAS_NO_ERROR) {
	return error;
    }
    
    uint8_t control;
    error = dallasReadRAM(0x201, 1, &control);
    if (error |= DALLAS_NO_ERROR) {
	return error;
    }
    control |= 1 << 4;
    error = dallasWriteRAM(0x201, 1, &control);
    if (error |= DALLAS_NO_ERROR) {
	return error;
    }
    return DALLAS_NO_ERROR;
}
#endif

uint32_t dallasTimeRead() {
    u08 i;
    uint32_t ret = 0;
    i = dallasReset();
    if (i != DALLAS_PRESENCE) {
	return i;
    }
    dallasWriteByte(DALLAS_SKIP_ROM);
    dallasWriteByte(0xf0);
    dallasWriteByte(0x2);
    dallasWriteByte(0x2);
    dallasReadByte(); // LSB
    for (int i = 0; i < 4; i++) {
	uint32_t c = dallasReadByte();
	ret |= c << (i*8);
    }
    return ret;
}

u08 dallasReadROM(dallas_rom_id_T* rom_id)
{
	u08 i;

	// reset the 1-wire bus and look for presence
	i = dallasReset();
	if (i != DALLAS_PRESENCE)
		return i;
	
	// send READ ROM command
	dallasWriteByte(DALLAS_READ_ROM);

	// get the device's ID 1 byte at a time
	for(i=0;i<8;i++)
		rom_id->byte[i] = dallasReadByte();

	return DALLAS_NO_ERROR;
}

#ifdef TABLE_CRC
u08 dallasAddressCheck(dallas_rom_id_T* rom_id)
{
    u08 i;

    dallas_crc = 0;
    for(i = 0; i < 7; i++)
    {
	uint8_t byte = rom_id->byte[i];
	dallas_crc = dallas_crc_table[dallas_crc^byte];
    }
    if (dallas_crc != rom_id->byte[DALLAS_CRC_IDX]) {
	return DALLAS_CRC_ERROR;
    }
    return DALLAS_NO_ERROR;
}
#else
u08 dallasAddressCheck(dallas_rom_id_T* rom_id)
{
    uint8_t i, t, crc;
    crc = 0;
    for(i = 0; i < 7; i++)
    {
	uint8_t byte = rom_id->byte[i];
	for (t = 0; t < 8; ++t) {
	    uint8_t u = (byte & 1) ^ (crc & 1);
	    crc = (crc >> 1) ^ (0x8c * u);
	    byte >>= 1;
	}
    }
    if (crc != rom_id->byte[DALLAS_CRC_IDX]) {
	return DALLAS_CRC_ERROR;
    }
    return DALLAS_NO_ERROR;
}
#endif // TABLE_CRC
