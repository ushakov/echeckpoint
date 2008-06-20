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

#include "filesys.h"
#include "priority_queue.h"

uint8_t buf[512];

void dump_flash (uint32_t size) {
    uint32_t off = 0;
    while (off < size) {
	putHexF(off, 8);
	putProg(": ");
	flash_read(off, buf, 16);
	for (int t = 0; t < 16; ++t) {
	    if (off + t < size) {
		putProg(" ");
		putHexF(buf[t], 2);
	    }
	}
	putCRLF();
	off += 16;
    }
}

void dump_dir_block (uint32_t addr) {
    uint16_t len;
    flash_read(addr, (uint8_t *) &len, 2);
    putProg("Dir block at: ");
    putHexF(addr, 8);
    putProg(" len: ");
    putInt(len);
    putCRLF();

    uint32_t next = 0;
    flash_read(addr+2, (uint8_t *) &next, 3);
    putProg("Next dir at: ");
    putHexF(next, 8);
    putCRLF();

    for (uint8_t i = 0 ; i < 100; ++i) {
	uint32_t info;
	flash_read(addr + 5 + 4*i, (uint8_t *) &info, 4);
	if (info == 0) continue;
	putHexF(addr + 5 + 4*i, 8);
	putProg(": block info ");
	putHexF(info, 8);
	putCRLF();
    }
}

void dump_cp_block (uint32_t addr) {
    uint16_t len;
    flash_read(addr, (uint8_t *) &len, 2);
    putProg("CP block at: ");
    putHexF(addr, 8);
    putProg(" len: ");
    putInt(len);
    putProg(" events: ");
    putInt((len - 6) / 9);
    putCRLF();

    uint16_t cp = 0;
    uint16_t num = 0;
    flash_read(addr + 3, (uint8_t *) &cp, 1);
    flash_read(addr + 4, (uint8_t *) &num, 2);
    putProg("CP: ");
    putInt(cp);
    putProg(" #");
    putInt(num);
    putCRLF();

    for (uint16_t off = 6 ; off < len; off += 9) {
	uint8_t buf[9];
	flash_read(addr + off, buf, 9);
	putProg("Ev:");
	for (int i = 0; i < 9; ++i) {
	    putProg(" ");
	    putHexF(buf[i], 2);
	}	    
	putCRLF();
    }
}


void fake_block (int len, int cp, int num, uint8_t tr) {
    *(uint16_t *) buf = 6 + len*9;
    *(buf+2) = cp;
    *(uint16_t *) (buf + 3) = num;
    uint32_t dir = filesys_write_block(buf);
    flash_write(dir+3, &tr, 1);
    pq_add(dir);
}

#define Q_LEN (0x200000UL - 3)
#define Q_START (Q_LEN - 3)

#define ADDR(x) (Q_START - (x)*3)


void main() {
    uart_init();
    uart_set_baud_rate(115200);
    putProg("Welcome to Checkpoint Debug!"); putCRLF();
    flash_init();
    filesys_init();
    pq_init();

    // write a couple of blocks for a couple of CPs
    fake_block(4, 0, 0, 2);
    fake_block(5, 0, 1, 3);
    fake_block(3, 0, 2, 5);
    
    fake_block(2, 1, 0, 2);
    fake_block(2, 1, 1, 1);
    fake_block(5, 1, 10, 0);

    flash_commit();

    dump_flash(768);

    uint32_t addr = 768;
    do {
	uint16_t len;
	flash_read (addr, (uint8_t *) &len, 2);
	if (len == 0 || len == 0xffff) break;
	if (len == 405) {
	    dump_dir_block(addr);
	} else {
	    dump_cp_block(addr);
	}
	addr += len;
    } while(1);

    putProg("PQ START:\r\n");
    uint16_t s = pq_len();
    for (int i = 0; i < s; ++i) {
	uint32_t dir = 0;
	uint32_t info = 0;
	flash_read(ADDR(i), (uint8_t *) &dir, 3);
	flash_read(dir, (uint8_t *) &info, 4);
	putInt(i);
	putProg(": ");
	putHexF(dir, 8);
	putProg(" i=");
	putHexF(info, 8);
	putCRLF();
    }

    pq_vis_start();
    for (int i = 0; i < 5; ++i) {
	uint32_t dir = pq_peek_top();
	uint8_t tr;
	flash_read(dir+3, &tr, 1);
	tr ++;
	flash_write(dir+3, &tr, 1);
	pq_vis_next();
    }

    putProg("PQ IN VIS:\r\n");
    s = pq_len();
    for (int i = 0; i < s; ++i) {
	uint32_t dir = 0;
	uint32_t info = 0;
	flash_read(ADDR(i), (uint8_t *) &dir, 3);
	flash_read(dir, (uint8_t *) &info, 4);
	putInt(i);
	putProg(": ");
	putHexF(dir, 8);
	putProg(" i=");
	putHexF(info, 8);
	putCRLF();
    }

    pq_vis_end();

    putProg("PQ END:\r\n");
    s = pq_len();
    for (int i = 0; i < s; ++i) {
	uint32_t dir = 0;
	uint32_t info = 0;
	flash_read(ADDR(i), (uint8_t *) &dir, 3);
	flash_read(dir, (uint8_t *) &info, 4);
	putInt(i);
	putProg(": ");
	putHexF(dir, 8);
	putProg(" i=");
	putHexF(info, 8);
	putCRLF();
    }
    
    while(1) { }
}
