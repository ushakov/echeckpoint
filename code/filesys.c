#include "global.h"
#include "config.h"

#include "eeprom.h"
#include "flash.h"

static uint32_t cur_mem_end;

static void fill_zeroes(uint32_t addr, uint16_t size) {
    uint8_t buf[16];
    for (int i = 0; i < 16; ++i) {
	buf[i] = 0;
    }
    while (size > 0) {
	uint8_t to_write = 16;
	if (to_write > size) to_write = size;
	flash_write (addr, buf, to_write);
	size -= to_write;
	addr += to_write;
    }
}

void filesys_init() {
    cur_mem_end = 0;
    cur_mem_end = eeprom_read(EEP_FILE_MEMEND_2);
    cur_mem_end <<= 8;
    cur_mem_end |= eeprom_read(EEP_FILE_MEMEND_1);
    cur_mem_end <<= 8;
    cur_mem_end |= eeprom_read(EEP_FILE_MEMEND_0);
}

void filesys_set_cur_mem_end(uint32_t new_end) {
    cur_mem_end = new_end;
    eeprom_write(EEP_FILE_MEMEND_0, cur_mem_end & 0xff);
    eeprom_write(EEP_FILE_MEMEND_0, (cur_mem_end >> 8) & 0xff);
    eeprom_write(EEP_FILE_MEMEND_0, (cur_mem_end >> 16) & 0xff);
}

void filesys_reset() {
    filesys_set_cur_mem_end(768);
    fill_zeroes(0, 768);
}

// add a new dir block linked after the specified one (if after != 0)
// return address of the new block
static uint32_t add_dir_block(uint32_t after) {
    uint32_t addr = cur_mem_end;
    if (after != 0) {
	flash_write (after + 2, (uint8_t *) &addr, 3);
    }
    uint16_t len = 405;  // length of dir block : 2b len, 3b next link, 4*100 data
    flash_write(cur_mem_end, (uint8_t *) &len, 2);
    fill_zeroes(cur_mem_end + 2, len - 2);
    filesys_set_cur_mem_end(cur_mem_end + len);
    return addr;
}

// find the address of dir entry for this block
// dir entry is uint32_t, high byte is transmission count, low 3 bytes
// is address
uint32_t filesys_get_dir_entry(uint16_t cp, uint16_t num) {
    uint32_t dir = 0;
    flash_read (cp*3, (uint8_t *) &dir, 3);
    if (dir == 0) {
	dir = add_dir_block(0);
	flash_write(cp*3, (uint8_t *) &dir, 3);
    }
    while (num > 100) {
	uint32_t new_dir = 0;
	flash_read(dir+2, (uint8_t *) &new_dir, 3);
	if (new_dir == 0) {
	    new_dir = add_dir_block(dir);
	}
	num -= 100;
	dir = new_dir;
    }
    return dir + 5 + num*4;
}

// write event block stored at address addr
// use info in the block to determine its CP and number, as well as its length
// return the address of directory entry for the block
// !!! do nothing if we already have this block! (still return dir entry addr)
uint32_t filesys_write_block(uint8_t *addr) {
    uint16_t len = * (uint16_t *) addr;
    uint8_t cp = *(addr+2);
    uint16_t num = * (uint16_t *) (addr+3);
    uint32_t dir = filesys_get_dir_entry(cp, num);
    uint32_t meta = 0;
    flash_read(dir, (uint8_t *) &meta, 4);
    if (meta != 0) {
	return dir;  // we alreday have it
    }
    // prepare and write metadata
    meta = cur_mem_end;  // high byte 0 means 0 transmissions yet
    flash_write (dir, (uint8_t *) &meta, 4);
    flash_write (cur_mem_end, addr, len);
    filesys_set_cur_mem_end(cur_mem_end + len);
    return dir;
}
