#ifndef MISC_H
#define MISC_H

void put_int (int number, char *c);

#define nop() do { __asm__ __volatile__ ("nop" "\n\t" "nop" ); } while (0)


#endif /* MISC_H */
