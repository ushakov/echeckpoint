#ifndef CP_H
#define CP_H

#include "global.h"

#include <inttypes.h>
#include "dallas.h"

void cp_init();
void cp_reset(uint16_t new_mmbid);
void cp_register(dallas_rom_id_T *rom);

extern uint16_t mmbid;
extern uint8_t cpid;
extern uint16_t current_no;

extern uint8_t buf[512];

#endif /* CP_H */
