#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

#define MAX(x, y) ((x) > (y)) ? (x) : (y)
#define MIN(x, y) ((x) < (y)) ? (x) : (y)

// stable: T1H: 12, T0H: 3, T1_TOP: 23, T0_TOP: 17

// orig: 14, last: 10
#define T1H_COUNT 13
// orig: 6, last: 5
#define T0H_COUNT 4
// orig: 23
#define T1_TOP 25
// orig: 16, last: 18
#define T0_TOP 16

#define LED_HBIT()         \
    do                     \
    {                      \
        OCR0A = T1_TOP;    \
        OCR0B = T1H_COUNT; \
    } while (0)
#define LED_LBIT()         \
    do                     \
    {                      \
        OCR0A = T0_TOP;    \
        OCR0B = T0H_COUNT; \
    } while (0)

#define TIM0_START() TCCR0B |= (1 << CS00)
#define TIM0_STOP() TCCR0B &= ~(1 << CS00)

#define LED_SENDBYTE(ledbyte)      \
    do                             \
    {                              \
        brightness = ledbyte;      \
        i = 0;                     \
        LED_SETBIT(ledbyte & 128); \
        TCNT0 = 0;                 \
        TIM0_START();              \
                                   \
    } while (0)

#define LED_SETBIT(logic) \
    do                    \
    {                     \
        if (logic)        \
        {                 \
            LED_HBIT();   \
        }                 \
        else              \
        {                 \
            LED_LBIT();   \
        }                 \
        i++;              \
    } while (0)

#define LED_WAITBYTE() while (i < 8)
#define LED_WAIT() while (!(i & ledbit_threshold_mask))

            volatile uint8_t brightness = 0x00;
volatile uint8_t i = 0;

volatile uint8_t count = 0, limit = 2, window = 2;

volatile uint8_t ledbit_threshold_mask = 0xF8;

ISR(TIMER0_COMPB_vect, ISR_BLOCK)
{
    if (!(i & ledbit_threshold_mask))
    {
        LED_SETBIT((brightness << i) & 128);
    }
    else {
        TIM0_STOP();
        i++;
    }
}

void setup(void)
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
    setup();

    uint8_t color = 0x0F;
    while (1)
    {
        LED_SENDBYTE(color);
        LED_WAIT();
        count++;
        if (count >= limit)
        {
            //PORTB |= (1 << PORTB5);
            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));

            TCCR0A &= ~(1 << COM0B1);
            i = 0;
            ledbit_threshold_mask = 0xC0;

            // delay
            for (count = 0; count < 150; count++)
            {
                LED_SENDBYTE(0);
                LED_WAIT();
            }

            ledbit_threshold_mask = 0xF8;
            TCCR0A |= (1 << COM0B1);

            count = 0;
            TCCR0A |= (1 << COM0B1);

            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));

            limit++;
        }
        if (limit >= 60) {
            limit = 0;
            color ^= 0x0F;
        }
    }
}