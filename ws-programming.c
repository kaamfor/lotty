#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>

// orig: 49; last: 60
#define T1H_COUNT 14
// orig: 25; last: 15
#define T0H_COUNT 6

#define LED_HBIT() OCR0B = T1H_COUNT
#define LED_LBIT() OCR0B = T0H_COUNT
#define TIM0_EN() TCCR0B |= (1 << CS00)
#define TIM0_DIS() TCCR0B &= ~(1 << CS00)

volatile uint8_t brightness = 0x00;
volatile uint8_t i = 0;

volatile uint8_t count = 0, limit = 10, window = 10;

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
    }
    i++;
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
    OCR0A = 22;
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

    uint8_t j;
    uint8_t selected_byte = 0;
    uint8_t last_selected_byte = selected_byte;

    
    while (1)
    {
        // Select LED byte via ADC value
        ADCSRA |= (1 << ADSC);

        while (ADCSRA & (1 << ADSC))
        {
            //asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));
        }

        // dummy read - is this neccessary?
        selected_byte = ADCL;
        selected_byte = ADCH;
        
        if (selected_byte > 128)
        {
            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PORTB)), "I"(PORTB5));
        }
        else
        {
            asm("cbi %0,%1" ::"I"(_SFR_IO_ADDR(PORTB)), "I"(PORTB5));
        }

        //while (selected_byte);

        //selected_byte = (selected_byte+1)%60;
        
        // send 0's
        brightness = 0;
        i = 0;
        TIM0_EN();
        for (j = 0; j < selected_byte; j++)
        {
            while (i < 8);
            i = 0;
        }
        TIM0_DIS();

        // send 1
        brightness = 0x00;
        i = 0;
        TIM0_EN();
        while (i < 8);
        TIM0_DIS();

        // send 0's
        brightness = 0;
        i = 0;
        TIM0_EN();
        for (j = selected_byte; j <= last_selected_byte; j++)
        {
            while (i < 8);
            i = 0;
        }
        TIM0_DIS();

        last_selected_byte = selected_byte;
        

        TCCR0A &= ~(1 << COM0B1);
        TIMSK0 &= ~(1 << OCIE0B);
        TIM0_EN();

        // delay
        for (count = 0; count < 100;)
        {
            // asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));
            count += (TIFR0 & (1 << OCF0B)) ? 1 : 0;
            // TIFR0 &= ~(1 << OCF0B);
            TIFR0 |= (1 << OCF0B);
        }
        TIM0_DIS();
        TIMSK0 |= (1 << OCIE0B);
        TCCR0A |= (1 << COM0B1);
    }




    uint8_t top, bottom;
    uint8_t count_limit = limit*3, real_window = window*3;
    uint8_t brightness_high = 0x01;

    while (1)
    {
        brightness = brightness_high, top = 1, bottom = 0;

        while (top <= count_limit)
        {
            count = 0;
            i = 0;
            for (j = 0; j < top; j++)
            {
                if (top - real_window > 0 && top - j >= real_window)
                {
                    brightness = 0x00;
                }
                else
                {
                    brightness = brightness_high;
                }
                TIM0_EN();
                //TCCR0B |= (1 << CS01);
                while (i < 8);
                TIM0_DIS();
                //TCCR0B &= ~(1 << CS01);
                i = 0;
            }

            TCCR0A &= ~(1 << COM0B1);
            TIMSK0 &= ~(1 << OCIE0B);
            TIM0_EN();

            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));

            // delay
            for (count = 0; count < 253;)
            {
                // asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));
                count += (TIFR0 & (1 << OCF0B)) ? 1 : 0;
                // TIFR0 &= ~(1 << OCF0B);
                TIFR0 |= (1 << OCF0B);
            }
            TIM0_DIS();
            TIMSK0 |= (1 << OCIE0B);
            TCCR0A |= (1 << COM0B1);

            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));

            top++;
        }

        TCCR0A &= ~(1 << COM0B1);
        TIM0_EN();
        for (j = 0; j < 254; j++)
        {
            while (i < 254);
            i = 0;
        }
        TIM0_DIS();
        TCCR0A |= (1 << COM0B1);
        

        // ADC TOP set
        /*ADCSRA = (1 << ADEN) | (1 << ADSC);

        while (ADCSRA & (1 << ADIF))
        {
            asm("sbi %0,%1" ::"I"(_SFR_IO_ADDR(PINB)), "I"(DDB5));
        }
        brightness_high = ADCH;

        ADCSRA = (1 << ADIF);
        */
    }

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
            //brightness = (count == selected) ? 0x0F : 0x00;
            brightness = 0x00;
            i = 0;
        }
    }
}