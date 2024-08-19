#ifndef _LOTTY_TWI_C_
#define _LOTTY_TWI_C_
#include "common.h"
#include "twi.h"

#include <avr/io.h>
#include <util/twi.h>

/* Store the last successful operation
*/
struct
{
    uint8_t has_error;
    uint8_t sreg; // updated on comm. failure
    uint8_t is_read_op;
    uint8_t is_closed;
    uint8_t data_transmitted; // n means at least n-1 bytes has been transmitted successfully
    uint8_t restart_count;
} last_transmission = {0, 0, 0, 0, 0, 0};


/* Return 0 if transmission completed without error
 */
uint8_t twi_has_error(void)
{
    return last_transmission.has_error;
}

uint8_t twi_transferred_bytes()
{
    return last_transmission.data_transmitted;
}

uint8_t twi_last_sreg()
{
    return last_transmission.sreg;
}

uint8_t twi_restart_count()
{
    return last_transmission.restart_count;
}

uint8_t twi_recover(void)
{

}

void start_transmit(uint8_t slave_addr, uint8_t is_read_op)
{
    uint8_t read_bit = (is_read_op ? 0b00000001 : 0b00000000);
    uint8_t sreg_addr_condition = is_read_op ? TW_MR_SLA_ACK : TW_MT_SLA_ACK;
    
    // Start condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    loop_until_bit_is_set(TWCR, TWINT);
    // condition check
    if ((TWSR & 0xF8) == TW_START || (TWSR & 0xF8) == TW_REP_START)
    {
        if ((TWSR & 0xF8) == TW_REP_START)
        {
            last_transmission.restart_count++;
        }
        else
        {
            last_transmission.restart_count = 0;
        }

        // Address slave
        TWDR = slave_addr << 1 + read_bit;
        TWCR = (1 << TWINT) | (1 << TWEN);
        loop_until_bit_is_set(TWCR, TWINT);

        // check ACK
        if ((TWSR & 0xF8) == sreg_addr_condition)
        {
            last_transmission.has_error = 0;
            last_transmission.sreg = TWSR;
            last_transmission.is_read_op = is_read_op;
            last_transmission.is_closed = 0;
            last_transmission.data_transmitted = 0;
            return;
        }
    }
    last_transmission.has_error = 1;
    last_transmission.sreg = TWSR;
    last_transmission.is_read_op = is_read_op;
    last_transmission.is_closed = 0;
    last_transmission.data_transmitted = 0;
}

void twi_read_singlebyte_blocking(twi_Locate locate, uint8_t *data)
{
    if (last_transmission.has_error)
    {
        twi_recover();
    }
    start_transmit(locate.slave_addr, 1);
    if (last_transmission.has_error)
    {
        last_transmission.sreg = TWSR;
        last_transmission.is_closed = 0;
        return;
    }
    
    // Initiate read and wait
    TWCR &= ~(1 << TWEA); // NACK
    TWCR |= (1 << TWINT) | (1 << TWEN);
    loop_until_bit_is_set(TWCR, TWINT);
    // check ACK
    if ((TWSR & 0xF8) != TW_MR_DATA_ACK)
    {
        last_transmission.has_error = 1;
        last_transmission.sreg = TWSR;
        last_transmission.is_closed = 0;
        return;
    }
    *data = TWDR;
    last_transmission.data_transmitted++;

    // Stop condition
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    last_transmission.is_closed = 1;
}

void twi_write_singlebyte_blocking(twi_Locate locate, uint8_t data)
{
    if (last_transmission.has_error)
    {
        twi_recover();
    }
    start_transmit(locate.slave_addr, 0);
    if (last_transmission.has_error)
    {
        last_transmission.sreg = TWSR;
        last_transmission.is_closed = 0;
        return;
    }

    // Initiate write
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    last_transmission.data_transmitted++;

    loop_until_bit_is_set(TWCR, TWINT);
    // check ACK
    if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
    {
        last_transmission.has_error = 1;
        last_transmission.sreg = TWSR;
        last_transmission.is_closed = 0;
        return;
    }

    // Stop condition
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    last_transmission.is_closed = 1;
}

void twi_read_stream_blocking(twi_Locate locate, uint8_t *data, uint8_t data_no)
{
    uint8_t i, last_bit;

    // Receive multiple byte
    if (last_transmission.has_error)
    {
        twi_recover();
    }
    start_transmit(locate.slave_addr, 1);
    if (last_transmission.has_error)
    {
        last_transmission.sreg = TWSR;
        last_transmission.is_closed = 0;
        return;
    }

    for (i = 0; i < data_no; i++)
    {
        last_bit = i == (data_no - 1);

        // Initiate read and wait
        if (last_bit)
        {
            TWCR &= ~(1 << TWEA);
        }
        TWCR |= (1 << TWINT) | (1 << TWEN);

        loop_until_bit_is_set(TWCR, TWINT);
        // check ACK
        if ((TWSR & 0xF8) != TW_MR_DATA_ACK)
        {
            last_transmission.has_error = 1;
            last_transmission.sreg = TWSR;
            last_transmission.is_closed = 0;
            return;
        }
        data[i] = TWDR;
        last_transmission.data_transmitted++;
    }

    // Stop condition
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    last_transmission.is_closed = 1;
}

void twi_write_stream_blocking(twi_Locate locate, const uint8_t *data, uint8_t data_no)
{
    uint8_t i;

    if (last_transmission.has_error)
    {
        twi_recover();
    }
    start_transmit(locate.slave_addr, 0);
    if (last_transmission.has_error)
    {
        last_transmission.sreg = TWSR;
        last_transmission.is_closed = 0;
        return;
    }

    // Write bytestream
    loop_until_bit_is_set(TWCR, TWINT);
    for (i = 0; i < data_no; i++)
    {
        // Initiate write
        TWDR = data[i];
        TWCR = (1 << TWINT) | (1 << TWEN);
        last_transmission.data_transmitted++;

        loop_until_bit_is_set(TWCR, TWINT);
        // check ACK
        if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
        {
            last_transmission.has_error = 1;
            last_transmission.sreg = TWSR;
            last_transmission.is_closed = 0;
            return;
        }
    }

    // Stop condition
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    last_transmission.is_closed = 1;
}

void twi_read_registerblock_blocking(twi_LocateRegister locate, uint8_t *data, uint8_t data_no)
{
    twi_Locate locate_stream = {.slave_addr = locate.slave_addr};

    // Send (write) register address first, then ReStart (atomic operation)
    // --------------------
    if (last_transmission.has_error)
    {
        twi_recover();
    }
    start_transmit(locate.slave_addr, 0);
    if (last_transmission.has_error)
    {
        last_transmission.sreg = TWSR;
        last_transmission.is_closed = 0;
        return;
    }

    // Initiate register address write
    TWDR = locate.slave_addr;
    TWCR = (1 << TWINT) | (1 << TWEN);
    last_transmission.data_transmitted++;

    loop_until_bit_is_set(TWCR, TWINT);
    // check ACK
    if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
    {
        last_transmission.has_error = 1;
        last_transmission.sreg = TWSR;
        last_transmission.is_closed = 0;
        return;
    }
    // --------------------

    // Receive multiple byte / register data
    // ReStart condition
    twi_read_stream_blocking(locate_stream, data, data_no);
}

void twi_write_registerblock_blocking(twi_LocateRegister locate, const uint8_t *data, uint8_t data_no)
{
    uint8_t i;

    if (last_transmission.has_error)
    {
        twi_recover();
    }
    start_transmit(locate.slave_addr, 0);
    if (last_transmission.has_error)
    {
        last_transmission.sreg = TWSR;
        last_transmission.is_closed = 0;
        return;
    }
    
    // Send register address
    loop_until_bit_is_set(TWCR, TWINT);
    TWDR = locate.register_addr;
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Write bytestream
    loop_until_bit_is_set(TWCR, TWINT);
    for (i = 0; i < data_no; i++)
    {
        // Initiate write
        TWDR = data[i];
        TWCR = (1 << TWINT) | (1 << TWEN);
        last_transmission.data_transmitted++;

        loop_until_bit_is_set(TWCR, TWINT);
        // check ACK
        if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
        {
            last_transmission.has_error = 1;
            last_transmission.sreg = TWSR;
            last_transmission.is_closed = 0;
            return;
        }
    }

    // Stop condition
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    last_transmission.is_closed = 1;
}

void twi_read_register_blocking(twi_LocateRegister locate, uint8_t *data)
{
    twi_read_registerblock_blocking(locate, data, 1);
}

void twi_write_register_blocking(twi_LocateRegister locate, uint8_t data)
{
    uint8_t data_arr[] = {data};
    twi_write_registerblock_blocking(locate, &data_arr, 1);
}


#endif