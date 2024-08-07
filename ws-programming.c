#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>

// orig: 43
#define T1H_COUNT 49
// orig: 25
#define T0H_COUNT 25

#define LED_HBIT() OCR0B = T1H_COUNT
#define LED_LBIT() OCR0B = T0H_COUNT

volatile uint8_t brightness = 0x00;
volatile uint8_t i = 0;

volatile uint8_t count = 0;

ISR(TIMER0_COMPB_vect, ISR_BLOCK)
{
    if (!(i >> 3)) {
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

void setup(void) {
    // cli()
    //unsigned char sreg;

    //sreg = SREG;
    cli();
    PORTD |= (1 << PORTD5);

    // Timer0 setup
    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << WGM02) | (1 << CS01);
    OCR0A = 95;
    TCNT0 = 0;
    TIMSK0 = (1 << OCIE0B);

    // post-setup, first bit
    if ((brightness << i) & 128)
    { LED_HBIT(); }
    else
    { LED_LBIT(); }
    i++;
    PRR &= ~(1 << PRTIM0);

    DDRD |= (1 << DDD5);
    //SREG = sreg;
    sei();

    // LED setup
    DDRB |= (1 << DDB5);
    PORTB |= (1 << PORTB5);
}

int main()
{
    setup();

    while (1)
    {
        if (count >= 150)
        {
            PRR |= (1 << PRTIM0);
            PORTB |= (1 << PORTB5);
        }
        else if (i > 7)
        {
            i = 0;
            count++;
            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));
        }
    }
}