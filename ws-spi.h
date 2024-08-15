#ifndef _LOTTY_WS_SPI_H_
#define _LOTTY_WS_SPI_H_

#include "common.h"

// *****************************
//  Macro definitions
//  Review before use
// *****************************

#define DD_MOSI_OUT (1 << DDB3)
#define DD_SS_OUT (1 << DDB2)
#define DD_SCK_OUT (1 << DDB5)

#define PRR_SPI_DIS (1 << PRSPI)
#define PRR_ADC_DIS (1 << PRADC)
#define PRR_USART_DIS (1 << PRUSART0)

// TODO: RENAME to ROW_LENGTH
#define WS_LED_LINE_LENGTH 8
#define WS_LED_5050_COUNT (11 * 8 + 2)
#define WS_LED_MONO_LINE_BG_BRIGHTNESS 0

// *****************************
//  Struct definitions
// *****************************

typedef struct ws_MonoLineData
{
    uint8_t offset;
    uint8_t limit;
    uint8_t bgbrightness;
} ws_MonoLineData;

typedef struct ws_ColoredLineData
{
    uint8_t offset;
    uint8_t slope;
    uint8_t thickness;
    uint8_t line_limit;
    uint8_t line_color3[3];
} ws_ColoredLineData;

typedef struct ws_MonoSpiralData
{
    uint8_t offset;
    uint8_t slope;
    uint8_t bgbrightness;
    uint8_t thickness;
    uint8_t limit;
} ws_MonoSpiralData;



typedef struct ws_HubData
{
    uint8_t spacing;
    uint8_t offset;
    uint8_t line_limit;
    uint8_t thickness;
    uint8_t twist;
    uint8_t line_color3[3];
} ws_HubData;

// *****************************
//  Function prototypes
// *****************************

void ws_setup(void);
void ws_build_spi_sequence(uint8_t color_byte);
void ws_send_buffered_sequence(void);
void ws_send_spi_sequence(uint8_t color);

void ws_save_buffer_seq4(uint8_t output_buffer[4]);
void ws_load_buffer_seq4(uint8_t input_buffer[4]);
void ws_fill_N(uint8_t n);

/* Draw line over the surface of 'bottle' with rasterization
 * Uses the in-buffer (mono) color for foreground color
 *
 * Offsetting means the previous LEDs will be colored with bg color
 *
 * The caller has the responsibility to latch the sent color codes
 */
void ws_draw_line_blocking(ws_MonoLineData *parameters, uint8_t row_count);


/* Draw line over the surface of 'bottle' with rasterization
 * Uses the in-buffer (mono) color for foreground color
 *
 * Offsetting means the previous LEDs will be colored with bg color
 * Equation:
 * y = (slope) * x + offset
 * //Slope is counterclockwise mapped between 0 to 255° angle ????
 *
 * The caller has the responsibility to latch the sent color codes
 */
void ws_draw_spiral_blocking(ws_MonoSpiralData *parameters);

/* Draw line over the surface of 'bottle' with rasterization
 * Uses the given line_color3 for line color
 * The in-buffer (mono) color will be used for filling the background
 *
 * Offsetting means the previous LEDs will be colored with bg color
 * Equation:
 * y = (slope) * x + offset
 * 
 * The caller has the responsibility to latch the sent color codes
 */
// void ws_draw_line_color3_blocking(uint8_t offset, uint8_t slope, uint8_t thickness, uint8_t line_color3[3]);

// void ws_draw_hub_color3_blocking(uint8_t spacing, uint8_t offset, uint8_t line_limit, uint8_t thickness, uint8_t twist, uint8_t line_color3[3]);

// void ws_draw_concentric_color3_blocking(uint8_t line, uint8_t offset, uint8_t radius, uint8_t line_width, uint8_t thickness, uint8_t line_color3[3]);

#endif