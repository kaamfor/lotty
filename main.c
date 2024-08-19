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
 *
 * TODO: ADC input
 */
#ifndef _LOTTY_WS_MAIN_H_
#define _LOTTY_WS_MAIN_H_

#include "common.h"

#include "inc/anim.h"
#include "ws-spi.h"
#include "twi.h"
#include "uart.h"


// *****************************
//  Macro definitions
//  Review before use
// *****************************

#define PRR_ADC_DIS (1 << PRADC)
#define PRR_USART_DIS (1 << PRUSART0)

#define ATTITUDE_LINE_BRIGHTNESS 64
#define BACKGROUND_BRIGHTNESS 0
#define ATTITUDE_ANGLE_MAX_ROW 3
#define ATTITUDE_VERTICAL_LINE_CORRECTOR_OFFSET 2

uint8_t recvd_data;

// *****************************
//  ISR definitions
// *****************************

ISR(USART_RX_vect, ISR_BLOCK)
{
    recvd_data = UDR0;
    UDR0 = recvd_data;
}

// *****************************
//  Main program functions
// *****************************

/* Draw artificial attitude graphic similar to aircraft instrument
 * attitude parameter processing has an offset and saturation like this:
 *                 Y
 *                 ^
 *                 |
 * -------         |         -------
 *       -\        |       -/
 *         -\      |     -/
 *           -\    |   -/
 * ----------------+----------------> X
 */
void draw_gyro_horizon(signed int clockwise_attitude, uint8_t start_offset)
{
    uint8_t input = abs(clockwise_attitude);

    if (input < 5)
    {
        ws_MonoLineData line_data = {
            .offset = WS_LED_5050_COUNT - WS_LED_LINE_LENGTH,
            .limit = 0,
            .bgbrightness = BACKGROUND_BRIGHTNESS
        };
        ws_build_spi_sequence(ATTITUDE_LINE_BRIGHTNESS);
        ws_draw_line_blocking(&line_data, 1);
    }
    else if (input > 112)
    {
        ws_MonoLineData line_data = {
            .offset = 0,
            .limit = 0,
            .bgbrightness = BACKGROUND_BRIGHTNESS
        };
        ws_build_spi_sequence(ATTITUDE_LINE_BRIGHTNESS);
        ws_draw_line_blocking(&line_data, 1);
    }
    else
    {
        uint8_t slope = 0, limit = 0, offset = 0;

        signed int diff_from_hcenter = 64 - input;
        uint8_t abs_diff = abs(64 - input);

        slope = (clockwise_attitude > 0) ? 64 + clockwise_attitude : 192 + clockwise_attitude;

        if (abs_diff < 2)
        {
            offset = ATTITUDE_VERTICAL_LINE_CORRECTOR_OFFSET;
            limit = 0;
            slope = 128;
        }
        else if (diff_from_hcenter < 0)
        {
            offset = 0;
            limit = ATTITUDE_ANGLE_MAX_ROW * WS_LED_LINE_LENGTH - WS_LED_LINE_LENGTH/2;
        }
        else if (diff_from_hcenter > 0)
        {
            offset = WS_LED_5050_COUNT - ATTITUDE_ANGLE_MAX_ROW * WS_LED_LINE_LENGTH;
        }

        if (clockwise_attitude < 0)
        {
            if (diff_from_hcenter < 0)
            {
                offset += 2 * ATTITUDE_VERTICAL_LINE_CORRECTOR_OFFSET;
            }
        }

        ws_MonoSpiralData line_data = {
            .offset = offset,
            .slope = slope,
            .bgbrightness = BACKGROUND_BRIGHTNESS,
            .thickness = 1,
            .limit = limit
        };
        ws_build_spi_sequence(ATTITUDE_LINE_BRIGHTNESS);
        ws_draw_spiral_blocking(&line_data);
        ws_build_spi_sequence(line_data.bgbrightness);
        ws_fill_N(WS_LED_5050_COUNT - line_data.limit);
    }
}

void main_setup(void)
{
    cli();

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
    main_setup();
    ws_setup();

    uint8_t color_byte;
    uint8_t c = 0, j = 1;
    while (1)
    {
        for(uint8_t i = 0; i < 5; i++)
        {
            color_byte = (j ? 0b10101010 : 0b01010101) - c;
            ws_send_spi_sequence(color_byte);
            ws_send_spi_sequence(~color_byte);
            ws_send_spi_sequence(color_byte);
            j = i+c;
            j %= 12;
        }
        c++;
        _delay_ms(1);
    }
}

#endif
