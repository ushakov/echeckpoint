#include "global.h"

#include "flash.h"

// the queue grows "downwards" -- from the highest address in flash
#define Q_LEN (0x200000UL - 3)
#define Q_START (Q_LEN - 3)

#define ADDR(x) (Q_START - (x)*3)

#define PARENT(x) (((x + 1) >> 1) - 1)
#define LEFT(x) ((x) * 2 + 1)
#define RIGHT(x) ((x) * 2 + 2)
#define ISROOT(x) ((x) == 0)

uint32_t len;
uint32_t vis_len;

static void pq_get(uint32_t num, uint32_t *dir, uint32_t *info) {
    *dir = 0;
    *info = 0;
    flash_read(ADDR(num), (uint8_t *) dir, 3);
    flash_read(*dir, (uint8_t *) info, 4);
}

static void pq_put(uint32_t num, uint32_t dir) {
    flash_write(ADDR(num), (uint8_t *) &dir, 3);
}

void pq_init() {
    len = 0;
    vis_len = 0;
    flash_read(Q_LEN, (uint8_t *) &len, 3);
}

void pq_reset() {
    len = 0;
    vis_len = 0;
    flash_write(Q_LEN, (uint8_t *) &len, 3);
}

static uint32_t bubble_up(uint32_t hole, uint8_t val) {
    while (!ISROOT(hole)) {
	uint32_t parent = PARENT(hole);
	uint32_t dir;
	uint32_t info;
	pq_get (parent, &dir, &info);
	if ((info >> 24) > val) {
	    pq_put(hole, dir);
	    hole = parent;
	} else {
	    break;
	}
    }
    return hole;
}

// add a directory entry at address dir
void pq_add(uint32_t dir) {
    uint32_t info;
    flash_read(dir, (uint8_t *) &info, 4);
    uint32_t cur = len;
    uint8_t tr = (info >> 24);
    cur = bubble_up(cur, tr);
    pq_put(cur, dir);
    len++;
    flash_write(Q_LEN, (uint8_t *) &len, 3);
}

// what's at the top? (does not change anything)
uint32_t pq_peek_top() {
    if (len > 0) {
	uint32_t dir, info;
	pq_get(0, &dir, &info);
	return dir;
    } else {
	return 0;
    }
}

// visiting the top of the queue; scheme:
// pq_vis_start();
// while (...) {
//   t = pq_peek_top();
//   if (is_ok(t)) {
//      (modify block(t) if needed)
//      pq_vis_next();  // top is temporary removed from queue
//   }
// }
// pq_vis_end();   // all visited entries are re-added to queue

void pq_vis_start() {
    vis_len = 0;
}

void pq_vis_next() {
    // 1. save the top
    // 2. push the hole down to the end of the heap
    // 3. fill the hole with the last value in the old heap
    // 4. bubble the new value up
    // 5. put the old top into the old last

    uint32_t topdir;
    uint32_t info;
    uint32_t dir;
    pq_get(0, &topdir, &info);

    len = len - 1;
    vis_len = vis_len + 1;

    uint32_t hole = 0;
    uint32_t next_child = 2;
    while (next_child < len) {
	uint32_t left_d;
	uint32_t left_i;
	pq_get(next_child, &dir, &info);
	pq_get(next_child - 1, &left_d, &left_i);
	if ((left_i >> 24) < (info >> 24)) {
	    next_child--;
	    dir = left_d;
	}

	pq_put(hole, dir);
	hole = next_child;
	next_child = 2 * (hole + 1);
    }
    if (next_child == len) {
	pq_get(next_child - 1, &dir, &info);
	pq_put(hole, dir);
	hole = next_child - 1;
    }

    pq_get (len, &dir, &info);
    hole = bubble_up (hole, (info >> 24));
    pq_put (hole, dir);
    pq_put (len, topdir);
}

void pq_vis_end() {
    for (uint16_t i = 0; i < vis_len; ++i) {
	uint32_t dir = 0;
	uint32_t info = 0;
	pq_get(len, &dir, &info);
	pq_add(dir);
    }
    vis_len = 0;
}

uint32_t pq_len() {
    return len;
}
