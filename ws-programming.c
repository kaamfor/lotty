#ifndef _LOTTY_COMMON_H_
#include "common.h"
#endif

#ifndef _LOTTY_WS_PROGRAMMING_H_

#define MAX(x, y) ((x) > (y)) ? (x) : (y)
#define MIN(x, y) ((x) < (y)) ? (x) : (y)

#define LED_1HCOUNT 13
#define LED_0HCOUNT 4
#define LED_1TOP 25
#define LED_0TOP 16

#define TIM0_START() TCCR0B |= (1 << CS00)
#define TIM0_STOP() TCCR0B &= ~(1 << CS00)

#define LED_SENDBYTE(ledbyte)         \
    do                                \
    {                                 \
        brightness = ledbyte;         \
        i = 0;                        \
        if (brightness & 128)         \
        {                             \
            t0_hcount = LED_1HCOUNT;  \
            t0_top = LED_1TOP;        \
        }                             \
        else                          \
        {                             \
            t0_hcount = LED_0HCOUNT;  \
            t0_top = LED_0TOP;        \
        }                             \
        LED_SETBIT();                 \
                                      \
        TCNT0 = 0;                    \
        TIM0_START();                 \
                                      \
        i++;                          \
        brightness <<= 1;             \
        t0_hcount = brightness & 128; \
                                      \
    } while (0)

#define LED_SETBIT()       \
    do                     \
    {                      \
        OCR0A = t0_top;    \
        OCR0B = t0_hcount; \
    } while (0)

#define LED_WAITBYTE() while (i < 8)
#define LED_WAIT() while (!(i & ledbit_threshold_mask))

volatile uint8_t brightness = 0x00;
volatile uint8_t t0_hcount, t0_top;
volatile uint8_t i = 0;

volatile uint8_t count = 0, limit = 2, window = 2;

volatile uint8_t ledbit_threshold_mask = 0xF8;

ISR(TIMER0_COMPB_vect, ISR_BLOCK)
{
    LED_SETBIT();
    TIM0_STOP();
    
    i++;
    brightness <<= 1;

    if (brightness & 128)
    {
        t0_hcount = LED_1HCOUNT;
        t0_top = LED_1TOP;
    }
    else
    {
        t0_hcount = LED_0HCOUNT;
        t0_top = LED_0TOP;
    }

    if (!(i & ledbit_threshold_mask))
    {
        TCNT0 = 0;
        TIM0_START();
    }
}

void wsProgramming_setup(void)
{
    // cli()
    // unsigned char sreg;

    // sreg = SREG;
    cli();
    PORTD |= (1 << PORTD5);

    // Timer0 setup
    PRR &= ~(1 << PRTIM0);
    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCNT0 = 0;
    TIMSK0 = (1 << OCIE0B);
    TCCR0B = (1 << WGM02);

    DDRD |= (1 << DDD5);
    // SREG = sreg;
    sei();

    // ADC setup
    ADCSRA = (1 << ADEN);

    DDRC &= ~(1 << DDC0);
    PORTC &= ~(1 << PORTC0);
    PRR &= ~(1 << PRADC);
    ADMUX = (1 << REFS0) | (1 << ADLAR);
    DIDR0 = (1 << ADC0D);
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // LED setup
    DDRB |= (1 << DDB5);
    PORTB &= ~(1 << PORTB5);
}

int main()
{
    wsProgramming_setup();

    uint8_t color = 0x0F;
    i = 0xFF; // needed by LED_WAIT()
    while (1)
    {
        // GRB
        LED_WAIT();
        LED_SENDBYTE(color);
        LED_WAIT();
        LED_SENDBYTE(0);
        LED_WAIT();
        LED_SENDBYTE(0);

        count++;
        if (count >= limit)
        {
            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));

            TCCR0A &= ~(1 << COM0B1);
            LED_WAIT();
            i = 0;
            ledbit_threshold_mask = 0xC0;
            i = 0xFF; // needed by LED_WAIT()
            TIM0_START();

            // delay for led latch
            for (count = 0; count < 150; count++)
            {
                LED_WAIT();
                TIM0_START();
                LED_SENDBYTE(0);
            }
            LED_WAIT();

            ledbit_threshold_mask = 0xF8;
            TCCR0A |= (1 << COM0B1);

            count = 0;
            TCCR0A |= (1 << COM0B1);

            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));

            limit++;
        }
        if (limit >= 15)
        {
            limit = 0;

            // Set color via ADC value
            ADCSRA |= (1 << ADSC);

            while (ADCSRA & (1 << ADSC))
                ;

            // dummy read - is this neccessary?
            color = ADCL;

            color = ADCH;

            if (color)
            {
                asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PORTB)), "I"(PORTB5));
            }
            else
            {
                asm("cbi %0,%1" ::"I"(_SFR_IO_ADDR(PORTB)), "I"(PORTB5));
            }
        }
    }
}

#endif
