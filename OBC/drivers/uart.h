#ifndef UART_H
#define UART_H

#define PORT_COM1 0x3F8

void uart_init();
int uart_received();
char uart_read();
int uart_transmit_empty();
void uart_write(char c);
void uart_print(char *str);
char uart_read_nonblocking();

#endif