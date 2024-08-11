/* Skeleton program with:
 * - WS821x LED string programming
 * - USART echo/null-modem emulator
 * 
 * The WS/Neopixel LED string output is attached to MOSI (PB3/17) pin,
 *  while the USART uses the appropiate TX+RX pins.
 * The LED string program in the main task controls five 5050 LEDs
 *  at high frequency with continuously changing brightness - PWM-like.
 * The SPI sends out two "bits" simultaneously, so it saves usable CPU cycles.
 * Although the SCK is not used externally, the simple-master mode
 *  requires it to be in output mode.
 * 
 * The USART ISR functions as a simple echo program at the moment
 *  to test the module setup and communication capabilities.
 * Current settings: 115200 baud, 8N1
 */

#ifndef _LOTTY_COMMON_H_
#include "common.h"
#endif

#ifndef _LOTTY_WS_PROGRAMMING_H_

#define MAX(x, y) ((x) > (y)) ? (x) : (y)
#define MIN(x, y) ((x) < (y)) ? (x) : (y)

#define DD_MOSI_OUT (1 << DDB3)
#define DD_SS_OUT (1 << DDB2)
#define DD_SCK_OUT (1 << DDB5)

#define PRR_SPI_DIS (1 << PRSPI)
#define PRR_ADC_DIS (1 << PRADC)
#define PRR_USART_DIS (1 << PRUSART0)

#define USART_HAS_DATA bit_is_set(UCSR0A, RXC0)
#define USART_READY bit_is_set(UCSR0A, UDRE0)


uint8_t i = 0;
uint8_t recvd_data;

ISR(USART_RX_vect, ISR_BLOCK)
{
    recvd_data = UDR0;
    UDR0 = recvd_data;
}

void wsProgramming_setup(void)
{
    cli();

    // SPI setup
    PRR &= ~PRR_SPI_DIS;
    SPCR = (1 << MSTR) | (1 << CPHA);

    DDRB |= DD_SCK_OUT | DD_MOSI_OUT | DD_SS_OUT;
    SPCR |= (1 << SPE);

    // ADC setup
    ADCSRA = (1 << ADEN);

    DDRC &= ~(1 << DDC0);
    PORTC &= ~(1 << PORTC0);
    PRR &= ~PRR_ADC_DIS;
    ADMUX = (1 << REFS0) | (1 << ADLAR);
    DIDR0 = (1 << ADC0D);
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // USART setup
    PRR &= ~PRR_USART_DIS;
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UPM00) | (1 << UCSZ01) | (1 << UCSZ00);

    #define BAUD 115200
    #include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    #if USE_2X
        UCSR0A |= (1 << U2X0);
    #else
        UCSR0A &= ~(1 << U2X0);
    #endif
    #undef BAUD

    // LED setup
    DDRB |= (1 << DDB5);
    PORTB &= ~(1 << PORTB5);

    sei();
}

int main()
{
    wsProgramming_setup();

    uint8_t c = 0, j = 1;
    while (1)
    {
        for(uint8_t i = 0; i < 60; i++)
        {
            SPDR = j ? 0b10001000 : 0b11101110;
            j = i+c;
            j %= 12;

            loop_until_bit_is_set(SPSR, SPIF);
        }
        c++;
        _delay_ms(1);
    }
}

#endif
