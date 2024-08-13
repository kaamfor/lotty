#ifndef _LOTTY_INC_ANIM_C_
#define _LOTTY_INC_ANIM_C_
#include "anim.h"

void unsigned2color3(uint8_t value, uint8_t color3[3])
{
    volatile uint8_t *inc_color, *dec_color, *null_color;
    uint8_t inc_color_i, dec_color_i, null_color_i;

    uint8_t output_value = value;
    uint8_t bitgroup = value >> 3;
    if (bitgroup >= 0b10101)
    {
        output_value = value - 168;

        inc_color_i = 0;
        dec_color_i = 2;
        null_color_i = 1;
        output_value = output_value * (128. / 88.);
    }
    else if (bitgroup >= 0b01010)
    {
        output_value = value - 80;

        inc_color_i = 2;
        dec_color_i = 1;
        null_color_i = 0;
        output_value = (output_value * (128. / 88.));
    }
    else
    {
        inc_color_i = 1;
        dec_color_i = 0;
        null_color_i = 2;
        output_value = output_value * (128. / 80.);
    }

    // Select color indexes
    inc_color = &color3[inc_color_i];
    dec_color = &color3[dec_color_i];
    null_color = &color3[null_color_i];

    // Assign color values

    *inc_color = output_value;
    *dec_color = 0b01111111 - output_value;
    *null_color = 0;
}

#endif