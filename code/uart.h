#ifndef UART_H
#define UART_H

void uart_init();
void uart_off();
void uart_set_baud_rate(uint32_t baudrate);
void uart_write_byte (uint8_t data);
int16_t uart_getchar();

#define UART_DOR 0x1
#define UART_FE  0x2
#define UART_PE  0x4
#define UART_BOR 0x8

#endif /* UART_H */
