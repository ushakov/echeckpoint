#include "global.h"

#include "dallas.h"

#include "flash.h"
#include "filesys.h"
#include "priority_queue.h"
#include "led.h"
#include "beep.h"
#include "eeprom.h"
#include "data.h"
#include "time.h"

// read from EEPROM, written by time setting routine
uint16_t mmbid;
// same, but written through serial monitor only
uint8_t cpid;

// We hold the current block at its place in flash
// and grow it if needed. WHen the carrier comes,
// we stop updating this block, leaving it in the last state.
// current_block is address in flash where the block starts,
// current_len is its length (in events).
// If there is no known current block, current_block == 0.
// Note that the state is consistent almost always
// because we update both the contents and the length
// at each step.
// current_no is number of this block at this CP
// if current_block == 0 then current_no is number of the next block to create
uint32_t current_block;
uint32_t current_dir;
uint16_t current_len;
uint16_t current_no;

// exchange buffer
uint8_t buf[512];
uint8_t last_id[6];

void cp_init() {
    mmbid = eeprom_read(EEP_MMBID_H);
    mmbid <<= 8;
    mmbid |= eeprom_read(EEP_MMBID_L);
    cpid = eeprom_read(EEP_CPID);
    current_block = 0;
    current_len = 0;
    current_no = eeprom_read(EEP_CURNO_H);
    current_no <<= 8;
    current_no |= eeprom_read(EEP_CURNO_L);
    for (int i = 0; i < 6; ++i) {
	last_id[i] = 0;
    }
}

void cp_reset(uint16_t new_mmbid) {
    filesys_reset();
    pq_reset();
    mmbid = new_mmbid;
    eeprom_write(EEP_MMBID_L, mmbid & 0xff);
    eeprom_write(EEP_MMBID_H, (mmbid >> 8) & 0xff);
    cpid = eeprom_read(EEP_CPID);
    current_no = 0;
    eeprom_write(EEP_CURNO_L, 0);
    eeprom_write(EEP_CURNO_H, 0);
}

static uint8_t block_crc(uint8_t *block) {
    uint16_t len = *(uint16_t *) block;
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; ++i) {
	if (i != 5) {  // skip #5, where crc is located
	    crc <<= 1;
	    crc ^= block[i];
	}
    }
    return crc;
}

void cp_close_block() {
    pq_add(current_dir);
    current_block = 0;
    current_no += 1;
    eeprom_write(EEP_CURNO_L, current_no & 0xff);
    eeprom_write(EEP_CURNO_H, (current_no >> 8) & 0xff);
}

void cp_register(dallas_rom_id_T *rom) {
    // is this the same button as just before?
    // (at the same time, remember this id in last_id)
    int8_t same = 1;
    for (int i = 0; i < 6; ++i) {
	if (last_id[i] != rom->byte[i+1]) {
	    same = 0;
	}
	last_id[i] = rom->byte[i+1];
    }
    if (same == 0) {
	// register the event:
	// 1. look if we already have current block and create one if necessary
	buf[0] = 6;
	buf[1] = 0;
	buf[2] = cpid;
	buf[3] = current_no & 0xff;
	buf[4] = (current_no >> 8) & 0xff;
	buf[5] = block_crc(buf);
	if (current_block == 0) {
	    current_dir = filesys_write_block(buf);
	    current_block = 0;
	    flash_read(current_dir, (uint8_t *) &current_block, 4);
	    current_block &= 0xffffff;
	    current_len = 0;
	}
	// 2. Create event and write it to current block
	uint32_t addr = 6 + 9*current_len;
	for (int i = 0; i < 6; ++i) {
	    buf[addr + i] = rom->byte[i + 1];
	}
	timestamp ts = time_get_ts();
	buf[addr + 6] = ts.sec;
	buf[addr + 7] = ts.min;
	buf[addr + 8] = ts.dayhour;
	addr += 9;
	current_len += 1;
	buf[0] = addr & 0xff;
	buf[1] = (addr >> 8) & 0xff;
	buf[5] = block_crc(buf);
	flash_write(current_block, buf, addr);
	// We have grown the last block in the filesys. We should
	// update cur_mem_end
	filesys_set_cur_mem_end(current_block + addr);
	// 3. If the block is full, close it
	if (current_len == 56) {
	    cp_close_block();
	}
    }
    // if the button is not carrier, finish
    if (rom->byte[DALLAS_FAMILY_IDX] == 0x01) goto return_ok;
    uint16_t memlen = 8192;

    // close current block
    if (current_block != 0) {
	cp_close_block();
    }

    // read the MMB id from the button
    uint16_t player_mmbid = 0;
    uint8_t res = dallasReadRAM(0, 2, (uint8_t *) &player_mmbid);
    if (res != DALLAS_NO_ERROR) goto return_err;
    if (player_mmbid == mmbid) {
	// get all the events from the button
	uint16_t addr = 2;
	while (1) {
	    uint16_t len;
	    res = dallasReadRAM(addr, 2, (uint8_t *) &len);
	    if (res != DALLAS_NO_ERROR) goto return_err;
	    if (len == 0) break;
	    res = dallasReadRAM(addr, len, buf);
	    if (res != DALLAS_NO_ERROR) goto return_err;
	    if (buf[5] != block_crc(buf)) goto return_err;
	    uint32_t new_dir = filesys_write_block(buf);
	    pq_add(new_dir);
	    addr += len;
	    time_check();
	}
    }
    // write our MMB id
    res = dallasWriteRAM(0, 2, (uint8_t *) &mmbid);
    if (res != DALLAS_NO_ERROR) return;
    // push events to the button
    uint16_t button_addr = 2;
    pq_vis_start();
    while(1) {
	uint32_t dir = pq_peek_top();
	if (dir == 0) {
	    goto end_loop;
	}
	uint32_t block = 0;
	uint8_t trans;
	flash_read(dir, (uint8_t *) &block, 4);
	trans = (block >> 24);
	block &= 0xffffff;
	uint16_t len;
	flash_read(block, (uint8_t *) &len, 2);
	if (len + button_addr > memlen - 2) {
	    goto end_loop;
	}
	flash_read(block, buf, len);
	res = dallasWriteRAM(button_addr, len, buf);
	if (res != DALLAS_NO_ERROR) {
	    pq_vis_end();
	    goto return_err;
	}
	trans++;
	flash_write(dir + 3, &trans, 1);
	button_addr += len;
	pq_vis_next();
	time_check();
    }
end_loop:
    pq_vis_end();
    // Add zero to button indicating end of record
    uint16_t zero = 0;
    res = dallasWriteRAM(button_addr, 2, (uint8_t *) &zero);
    if (res != DALLAS_NO_ERROR) {
	goto return_err;
    }
return_ok:    
    beep_yes();
    return;
return_err:
    beep_no();
    return;
}
