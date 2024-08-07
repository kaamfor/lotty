#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>

// orig: 49
#define T1H_COUNT 60
// orig: 25
#define T0H_COUNT 15

#define LED_HBIT() OCR0B = T1H_COUNT
#define LED_LBIT() OCR0B = T0H_COUNT

volatile uint8_t brightness = 0x00;
volatile uint8_t i = 0;

volatile uint8_t count = 0, limit = 1;

ISR(TIMER0_COMPB_vect, ISR_BLOCK)
{
    if (!(i >> 3))
    {
        if ((brightness << i) & 128)
        {
            LED_HBIT();
        }
        else
        {
            LED_LBIT();
        }
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
    OCR0A = 95;
    TCNT0 = 0;
    TIMSK0 = (1 << OCIE0B);
    TCCR0B = (1 << WGM02);

    // post-setup, first bit
    if ((brightness << i) & 128)
    {
        LED_HBIT();
    }
    else
    {
        LED_LBIT();
    }
    i++;

    DDRD |= (1 << DDD5);
    TCCR0B |= (1 << CS01); // enable clock
    // SREG = sreg;
    sei();

    // ADC setup
    PRR &= ~(1 << PRADC);
    ADMUX = (1 << ADLAR);
    DIDR0 = (1 << ADC0D);

    // LED setup
    DDRB |= (1 << DDB5);
    PORTB &= ~(1 << PORTB5);
}

int main()
{
    uint8_t selected = 0;

    setup();

    while (1)
    {
        /*if (!count) {
            // ADC TOP set
            ADCSRA = (1 << ADEN) | (1 << ADSC);

            while (ADCSRA & (1 << ADIF)) {
                asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));
            }
            uint8_t res = ADCH;

            ADCSRA = (1 << ADIF);
            OCR0B = res;
        }
        else */if (count >= limit)
        {
            //PORTB |= (1 << PORTB5);
            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));

            TIMSK0 &= ~(1 << OCIE0B);
            TCCR0A &= ~(1 << COM0B1);

            // delay
            for (count = 0; count < 253;)
            {
                //asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));
                count += (TIFR0 & (1 << OCF0B)) ? 1 : 0;
                //TIFR0 &= ~(1 << OCF0B);
                TIFR0 |= (1 << OCF0B);
            }
            count = 0;
            i = 0;
            TIMSK0 |= (1 << OCIE0B);
            TCCR0A |= (1 << COM0B1);

            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));
            //OCR0A++;

            limit++;
        }
        else if (i > 7)
        {
            count++;
            brightness = (count == selected) ? 0x0F : 0x00;
            i = 0;
        }
    }
}