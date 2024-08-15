#ifndef _LOTTY_WS_SPI_C_
#define _LOTTY_WS_SPI_C_

#include "common.h"

#include "inc/anim.h"
#include "ws-spi.h"

#define WS_SPIDATA(bit1, bit2)                        \
    do                                                \
    {                                                 \
        uint8_t out = bit2 ? 0b10001110 : 0b00001000; \
        if (bit1)                                     \
        {                                             \
            out += 0b01100000;                        \
        }                                             \
    } while (0)

/*
// Structure-backed global variable access is faster than non-structure one
// See AVR035 AppNote ; empirically correct by analyzing avr-objdump output
typedef struct
{
    volatile uint8_t ws_buffer;
    volatile uint8_t ws_next_color_code_i;
} ws_spi;
ws_spi global;
*/

// Uses RAM for performance reasons
const uint8_t ws_symbols[] = {0b10001000, 0b10001110, 0b11001000, 0b11001110};

volatile uint8_t ws_buffer, ws_color_codes[4], ws_next_color_code_i;

ISR(SPI_STC_vect, ISR_BLOCK)
{
    if (ws_next_color_code_i < 4)
    {
        SPDR = ws_buffer;
    }
    else
    {
        return;
    }
    ws_next_color_code_i++;
    ws_buffer = ws_color_codes[ws_next_color_code_i];
}

void ws_setup(void)
{
    unsigned char sreg;
    sreg = SREG;
    cli();

    // SPI setup
    PRR &= ~PRR_SPI_DIS;
    SPCR = (1 << MSTR) | (1 << CPHA);
    SPCR |= (1 << SPIE);

    DDRB |= DD_SCK_OUT | DD_MOSI_OUT | DD_SS_OUT;
    SPCR |= (1 << SPE);

    ws_next_color_code_i = 4;
    SREG = sreg;
}

void ws_build_spi_sequence(uint8_t color_byte)
{
    while (ws_next_color_code_i < 4);
    loop_until_bit_is_clear(SPSR, SPIF);

    // MSB first
    uint8_t code, i;
    for (i = 0; i < 4; i++)
    {
        code = (color_byte >> (6 - 2 * i)) & 0b00000011;
        ws_color_codes[i] = ws_symbols[code];
    }
}

void ws_send_buffered_sequence(void)
{
    // Guard current operation
    while (ws_next_color_code_i < 4);
    loop_until_bit_is_clear(SPSR, SPIF);

    ws_buffer = ws_color_codes[1];
    ws_next_color_code_i = 1;
    SPDR = ws_color_codes[0];
}

void ws_send_spi_sequence(uint8_t color_byte)
{
    ws_build_spi_sequence(color_byte);

    ws_send_buffered_sequence();
}

void ws_save_buffer_seq4(uint8_t output_buffer[4])
{
    uint8_t i;
    for (i = 0; i < 4; i++)
    {
        output_buffer[i] = ws_color_codes[i];
    }
}

void ws_load_buffer_seq4(uint8_t input_buffer[4])
{
    uint8_t i;
    for (i = 0; i < 4; i++)
    {
        ws_color_codes[i] = input_buffer[i];
    }
}

void ws_fill_N(uint8_t n)
{
    while (n)
    {
        ws_send_buffered_sequence();
        ws_send_buffered_sequence();
        ws_send_buffered_sequence();
        n--;
    }
}

void ws_draw_line_blocking(ws_MonoLineData *parameters, uint8_t row_count)
{
    uint8_t limit = parameters->limit ? MIN(parameters->limit, WS_LED_5050_COUNT) : WS_LED_5050_COUNT; // 90
    uint8_t line_end;
    uint8_t fg_color[4];

    line_end = parameters->offset + row_count * WS_LED_LINE_LENGTH; // 8
    if (line_end > limit)
    {
        line_end = limit;
    }

    ws_save_buffer_seq4(fg_color);
    ws_build_spi_sequence(parameters->bgbrightness); 
    if (parameters->offset > limit) // 0 > 90 ?
    {
        ws_fill_N(limit);
    }
    else
    {
        ws_fill_N(parameters->offset); // 0
        ws_load_buffer_seq4(fg_color);
        ws_fill_N(line_end - parameters->offset); // 8
        ws_build_spi_sequence(parameters->bgbrightness);
        ws_fill_N(limit - line_end); // 82
    }
}

/* Bresenham line drawing algorithm
 * Compute points with integer arithmetic
 */
void ws_draw_spiral_blocking(ws_MonoSpiralData *parameters)
{
    uint8_t offset, slope, bgbrightness, thickness;
    offset = parameters->offset;
    slope = parameters->slope;
    bgbrightness = parameters->bgbrightness;
    thickness = parameters->thickness;


    // i marks the current LED byte position
    // section_limit = upper limit of LEDs in each sections (multiples of 3 (<- 3 bytes per LED))
    uint8_t fg_color[4], block_limit, section_limit, i = 0;
    // save fg color and set bg color
    ws_save_buffer_seq4(fg_color);
    ws_build_spi_sequence(bgbrightness);

    block_limit = parameters->limit ? MIN(parameters->limit, WS_LED_5050_COUNT) : WS_LED_5050_COUNT;

    uint8_t x0, y0, x1;
    signed int y1;
    x0 = offset / WS_LED_LINE_LENGTH;
    y0 = offset % WS_LED_LINE_LENGTH;
    x1 = (block_limit / WS_LED_LINE_LENGTH) - 1;
    //y1 = (slope / WS_LED_LINE_LENGTH);

    y1 = slope - 0b10000000;

    signed int dx, dy, incrE, incrNE; //, x, y;
    signed long d;
    dx = x1 - x0;
    dy = y1 - y0;
    d = 2 * dy - dx;

    incrE = 2 * dy;
    incrNE = 2 * (dy - dx);

    section_limit = offset;
    while (i < block_limit)
    {
        // coloring background pixels
        for ( ; i < section_limit; i++)
        {
            // code duplicate because of uint8_t limits (and each LED programming consists of 3 bytes)
            ws_send_buffered_sequence();
            ws_send_buffered_sequence();
            ws_send_buffered_sequence();
        }

        // program foreground pixel
        ws_load_buffer_seq4(fg_color);
        ws_send_buffered_sequence();
        ws_send_buffered_sequence();
        ws_send_buffered_sequence();
        if (d <= 0)
        {
            if (y1 >= 0)
            {
                section_limit += WS_LED_LINE_LENGTH;
                d += incrE;
            }
            else
            {
                section_limit += WS_LED_LINE_LENGTH - 1;
                d += incrNE;
            }
        }
        else
        {
            if (y1 >= 0)
            {
                section_limit += WS_LED_LINE_LENGTH + 1;
                d += incrNE;
            }
            else
            {
                section_limit += WS_LED_LINE_LENGTH;
                d += incrE;
            }
        }
        ws_build_spi_sequence(bgbrightness);
        i++;
    }
}

#endif
