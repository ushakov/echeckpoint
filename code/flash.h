#ifndef FLASH_H
#define FLASH_H

#include <inttypes.h>

void flash_init();
void flash_activate();

// high level r/w access
void flash_read(uint32_t addr, uint8_t *mem, uint16_t len);
void flash_write(uint32_t addr, uint8_t *mem, uint16_t len);
void flash_commit();


#endif /* FLASH_H */
