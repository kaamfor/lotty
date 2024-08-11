#ifndef _LOTTY_COMMON_H_
#define _LOTTY_COMMON_H_

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#endif