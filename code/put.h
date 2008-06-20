#ifndef PUT_H
#define PUT_H

#include <avr/pgmspace.h>
#include <inttypes.h>

void putProgR(const char* str);
#define putProg(str) putProgR(PSTR(str))
void putRam(const char* str);
void putCRLF();
void putChar(char c);
void putInt(uint32_t number);
void putHex(uint32_t number);
void putRevHex(uint32_t number);
void putHexF(uint32_t number, uint8_t len);

#endif /* PUT_H */
