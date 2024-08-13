#ifndef _LOTTY_INC_ANIM_H_
#define _LOTTY_INC_ANIM_H_

#include <avr/io.h>

/* Compute an almost-continuous, circular color space from one 8-bit numeric input
 * |
 * |1.   2.   3.   1.     <- dominant color byte
 * |-    -    -   -
 * | \  / \  / \ /
 * |  \/   \/   /
 * |  /\   /\  / \
 * | /  \ /  \/   \
 * +---------------------
 * 0    80  168  256
 *
 * Two of the color bytes will be assigned non-zero value.
 *
 */
void unsigned2color3(uint8_t value, uint8_t color3[3]);

#endif