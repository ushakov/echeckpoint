#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

void eeprom_write (uint16_t where, uint8_t what);
uint8_t eeprom_read (uint16_t where);

// EEPROM MAP: TERMINAL
#define EEP_MAXPAGE_L 0
#define EEP_MAXPAGE_H 1

// EEPROM MAP: CHECKPOINT
#define EEP_MMBID_L 0
#define EEP_MMBID_H 1

#define EEP_FILE_MEMEND_0 2
#define EEP_FILE_MEMEND_1 3
#define EEP_FILE_MEMEND_2 4

#define EEP_CPID 5

#define EEP_CURNO_L 6
#define EEP_CURNO_H 7

#endif /* EEPROM_H */
