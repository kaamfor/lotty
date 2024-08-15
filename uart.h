#ifndef _LOTTY_UART_H_
#define _LOTTY_UART_H_

#include "common.h"

// *****************************
//  Function prototypes
// *****************************

#define UART_PUT_NEWLINE() \
    uart_putchar('\r');  \
    uart_putchar('\n');

void uart_putchar(uint8_t value);
void uart_putdigit_blocking(uint8_t value);
void uart_putstr_blocking(uint8_t *const nullterm_string);

#endif