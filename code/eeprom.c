#include <avr/io.h>
#include <avr/interrupt.h>

#include "eeprom.h"

void eeprom_write (uint16_t where, uint8_t what) {
    // wait until write is allowed
    while (EECR & (1<<EEWE)) ;

    // wait until flash programming is complete
    while (SPMCR & (1<<SPMEN)) ;

    EEAR = where;
    EEDR = what;

    cli();
    EECR |= (1<<EEMWE);
    EECR |= (1<<EEWE);
    sei();
}

uint8_t eeprom_read (uint16_t where) {
    /* Wait for completion of previous write */
    while(EECR & (1<<EEWE));
    /* Set up address register */
    EEAR = where;
    /* Start eeprom read by writing EERE */
    EECR |= (1<<EERE);
    /* Return data from data register */
    return EEDR;
}
