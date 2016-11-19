//============================================================================
//Weather Station Data Logger : Weather Shield for Arduino
//Revised by Weber Anderson, August 2010
// 
//This application is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 3 of the License, or (at your option) any later version.
//
//This application is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR When PARTICULAR PURPOSE. See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this library; if not, see <http://www.gnu.org/licenses/>
//
//=============================================================================
// this library has been enhanced for the needs of 
// the weather shield project for WSDL.
//
/*
extended_twi.h - TWI/I2C library for Wiring & Arduino
Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
Revised 9 June 2009 Christopher K. Johnson.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef extended_twi_h
#define extended_twi_h

#include <inttypes.h>

//#define ATMEGA8

/* Use F_CPU instead
#ifndef CPU_FREQ
#define CPU_FREQ 16000000L
#endif
*/

#ifndef TWI_FREQ
#define TWI_FREQ 100000L
#endif

#ifndef TWI_BUFFER_LENGTH
#define TWI_BUFFER_LENGTH 32
#endif

#define TWI_READY 0
#define TWI_MRX   1
#define TWI_MRXR  2
#define TWI_MTX   3
#define TWI_SRX   4
#define TWI_STX   5

#define TWI_STATUS_BUSY       0
#define TWI_STATUS_OK         1
#define TWI_STATUS_SLA_NACK   2
#define TWI_STATUS_DATA_NACK  3
#define TWI_STATUS_OTHER      4
#define TWI_STATUS_BAD_ARGS   5


uint8_t twi_error2status();

void twi_init();
void twi_enable_pullup_resistors(uint8_t);
void twi_setAddress(uint8_t);
long unsigned int twi_setSpeed(long unsigned int);

uint8_t twi_status();
uint8_t twi_getDataFromRead(uint8_t*, uint8_t); 
//
// these functions return TWI_STATUS values
//
uint8_t twi_beginReadFrom(uint8_t, uint8_t*, uint8_t);
uint8_t twi_beginReadFromAt(uint8_t, uint8_t, uint8_t*, uint8_t);
uint8_t twi_beginReadFromAt2(uint8_t,uint16_t, uint8_t*, uint8_t);
//
// these functions return the number of bytes read, or zero if there was an error
// twi_operationComplete() can be called to obtain the TWI_STATUS in case of error
//
uint8_t twi_readFrom(uint8_t, uint8_t*, uint8_t);
uint8_t twi_readFromAt(uint8_t, uint8_t, uint8_t*, uint8_t);
uint8_t twi_readFromAt2(uint8_t,uint16_t, uint8_t*, uint8_t);
//
// returns TWI_STATUS values
//
uint8_t twi_beginWriteTo(uint8_t, uint8_t*, uint8_t);
uint8_t twi_writeTo(uint8_t, uint8_t*, uint8_t);

uint8_t twi_transmit(uint8_t*, uint8_t);
void twi_attachSlaveRxEvent( void (*)(uint8_t*, int) );
void twi_attachSlaveTxEvent( void (*)(void) );
void twi_reply(uint8_t);
void twi_stop(void);
void twi_releaseBus(void);

#endif

