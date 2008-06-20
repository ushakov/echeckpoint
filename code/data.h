#ifndef DATA_H
#define DATA_H

typedef struct timestamp {
    uint8_t dayhour;    // bits 7..5 weekday, bits 4..0 hour
    uint8_t min;
    uint8_t sec;
} timestamp;

typedef struct team_info {
    uint16_t id;
    uint8_t button[6];
    timestamp ts;
} team_info;

// typedef for the rom IDs
// done so we can access the entire id or each individual byte
typedef union dallas_rom_id_U
{
    long long id;
    uint8_t byte[8];
} dallas_rom_id_T;

#endif /* DATA_H */
