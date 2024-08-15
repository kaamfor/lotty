#ifndef _LOTTY_UART_C_
#define _LOTTY_UART_C_
#include "uart.h"

void uart_putchar(uint8_t value)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = value;
}

void uart_putdigit_blocking(uint8_t value)
{
    uint8_t digit = '0';
    while (value >= 100)
    {
        digit++;
        value -= 100;
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = digit;

    digit = '0';
    while (value >= 10)
    {
        digit++;
        value -= 10;
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = digit;

    digit = '0';
    while (value >= 1)
    {
        digit++;
        value -= 1;
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = digit;
}

void uart_putstr_blocking(uint8_t *const nullterm_string)
{
    uint8_t i = 0;
    while (nullterm_string[i] != '\0')
    {
        loop_until_bit_is_set(UCSR0A, UDRE0);
        UDR0 = nullterm_string[i];
        i++;
    }
}

#endif