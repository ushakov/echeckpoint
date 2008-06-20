#include <stdio.h>

int main() {
    for (int i = 0; i < 256; ++i) {
	int s = i;
	int d = 0;
	for (int t = 0; t < 8; ++t) {
	    int u = (d & 1) ^ (s & 1);
	    d = (d >> 1) ^ (0x8c * u);
	    s >>= 1;
	    printf ("%x ", d);
	}
	printf("\n");
	s = i;
	int dd = 0;
	int s3 = s << 3;
	s3 |= (s3 & 0xff00) >> 8;
	s3 &= 0xff;
	
	int s4 = s << 4;
	s4 |= (s4 & 0xff00) >> 8;
	s4 &= 0xff;

	dd = s ^ s3 ^ s4;

	printf ("s: %x %x %x\n", s, s3, s4);
	printf ("%x\t%x\n", d, dd);
    }
}
