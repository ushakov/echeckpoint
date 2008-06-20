#ifndef TIME_H
#define TIME_H

#include <inttypes.h>
#include "data.h"

void time_init();
void time_check();
uint32_t time_get();
timestamp time_get_ts();
void time_set(uint32_t t); // in seconds since Monday

#endif /* TIME_H */
