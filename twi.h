#ifndef _LOTTY_TWI_H_
#define _LOTTY_TWI_H_

#include "common.h"

/* TWI module
 * This library handles the typical scenarios and tries not to be too generic.
 * The _blocking functions supports the master read and transmit modes.
 * 
 * Communication begins with TWCR setup and stops with the first NACK'd packet.
 * The functions are self-contained and takes care of Stop condition after
 *  a successful transfer, however, the caller's responsibility to check
 *  whether the transaction was successful or not.
 * An unchecked error means potential data loss. The lib functions can recover
 *  from transmission error, however.
 * 
 * _blocking functions are mostly failsafe operations.
*/

// *****************************
//  Struct definitions
// *****************************

typedef struct twi_Locate
{
    uint8_t slave_addr;
} twi_Locate;

typedef struct twi_LocateRegister
{
    uint8_t slave_addr;
    uint8_t register_addr;
} twi_LocateRegister;

// *****************************
//  Function prototypes
// *****************************

uint8_t twi_has_error(void);
uint8_t twi_transferred_bytes();
uint8_t twi_last_sreg();
uint8_t twi_restart_count();
void twi_read_singlebyte_blocking(twi_Locate locate, uint8_t *data);
void twi_write_singlebyte_blocking(twi_Locate locate, uint8_t data);
void twi_read_stream_blocking(twi_Locate locate, uint8_t *data, uint8_t data_no);
void twi_write_stream_blocking(twi_Locate locate, const uint8_t *data, uint8_t data_no);
void twi_read_register_blocking(twi_LocateRegister locate, uint8_t *data);
void twi_write_register_blocking(twi_LocateRegister locate, uint8_t data);
void twi_read_registerblock_blocking(twi_LocateRegister locate, uint8_t *data, uint8_t data_no);
void twi_write_registerblock_blocking(twi_LocateRegister locate, const uint8_t *data, uint8_t data_no);

#endif