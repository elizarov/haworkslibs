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
twi.c - TWI/I2C library for Wiring & Arduino
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

#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <compat/twi.h>

#ifndef noInterrupts
#define noInterrupts() cli()
#endif

#ifndef interrupts
#define interrupts() sei()
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define true 1
#define false 0

#include "extended_twi.h"

static volatile uint8_t twi_state;
static uint8_t twi_slarw;
static volatile uint8_t twi_slAtBytes;
static uint8_t twi_slAtLSB;
static uint8_t twi_slAtMSB;

static void (*twi_onSlaveTransmit)(void);
static void (*twi_onSlaveReceive)(uint8_t*, int);

static uint8_t* twi_masterBuffer;
static volatile uint8_t twi_masterBufferIndex;
static uint8_t twi_masterBufferLength;

static uint8_t* twi_txBuffer;
static volatile uint8_t twi_txBufferIndex;
static volatile uint8_t twi_txBufferLength;

static uint8_t* twi_rxBuffer;
static volatile uint8_t twi_rxBufferIndex;

static volatile uint8_t twi_error;

// Keep current baseline bit states for TWCR to which transient bits are or'ed
// when setting TWCR.  This allows desired on-bits in TWCR to be altered by
// functions rather than frozen in code. 
static volatile uint8_t twi_twcr_base;

/* 
* Function twi_init
* Desc     readys twi pins and sets twi bitrate
* Input    none
* Output   none
*/
void twi_init()
{
  noInterrupts();
  // initialize state
  twi_state = TWI_READY;

  // allocate buffers
  twi_masterBuffer = (uint8_t*) calloc(TWI_BUFFER_LENGTH, sizeof(uint8_t));
  twi_txBuffer = (uint8_t*) calloc(TWI_BUFFER_LENGTH, sizeof(uint8_t));
  twi_rxBuffer = (uint8_t*) calloc(TWI_BUFFER_LENGTH, sizeof(uint8_t));

  // set the twi clock rate for default bus speed
  twi_setSpeed(TWI_FREQ);

  // enable twi module, acks, and twi interrupt
  TWCR = twi_twcr_base = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
  interrupts();
}
//
// pullup resistors are not initially enabled in case there are already
// external pullups installed. this may be more important if level translation
// from 5V to 3V is setup, in which case enabling the pullups increase the
// risk of damaging hardware. use this function to enable internal pullup
// resistors if necessary.
//
void twi_enable_pullup_resistors(uint8_t enable_pullups)
{
  noInterrupts();
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega328P__)
  if (enable_pullups)
  {
    // activate internal pull-ups for twi
    // as per notef rom atmega8 manual pg167
    sbi(PORTC, 4);
    sbi(PORTC, 5);
  }
  else
  {
    cbi(PORTC, 4);
    cbi(PORTC, 5);
  }
#else
  if (enable_pullups)
  {
    // activate internal pull-ups for twi
    // as per note from atmega128 manual pg204
    sbi(PORTD, 0);
    sbi(PORTD, 1);
  }
  else
  {
    cbi(PORTD, 0);
    cbi(PORTD, 1);
  }
#endif
  interrupts();
}

/* 
* Function twi_setAddress
* Desc     sets slave address, relying on twi_init() to enable interrupt
* Input    address: 7bit i2c slave address we answer to
* Output   none
*/
void twi_setAddress(uint8_t address)
{
  noInterrupts();
  // set twi slave address (skip over TWGCE bit)
  TWAR = address << 1;
  // This should probably be changed to not force reset of TWGCE so
  // it could safely be set/reset by other functions (added later)
  // TWAR = ((address << 1) | (TWAR&_BV(TWGCE)));
  interrupts();
}

/* 
* Function twi_setSpeed
* Desc     sets twi hardware clock speed
* Input    bitsPerSecond: the speed (literals STANDARD, FAST also relevant)
* Output   the formerly set speed in bits per second
*/

long unsigned int twi_setSpeed(long unsigned int bitsPerSecond)
{
  /* twi bit rate formula from atmega128 manual pg 204
  SCL Frequency = CPU Clock Frequency / (16 + (TWBR * 2 * prescale))
  NOTE: TWBR should be 10 or higher for master mode
  It is 72 for a 16mhz Wiring board with 100kHz TWI */

  noInterrupts();
  // calculate the former bit rate to return
  uint8_t prescale = 1;
  if (TWSR & _BV(TWPS0)) { prescale = prescale << 2; }
  if (TWSR & _BV(TWPS1)) { prescale = prescale << 4; }
  long unsigned int formerSpeed = (F_CPU / (16 + (TWBR * 2 * prescale)));

  // initialize twi prescaler and bit rate
  cbi(TWSR, TWPS0);  // both bits cleared yields prescaler value of 1
  cbi(TWSR, TWPS1);
  TWBR = ((F_CPU / bitsPerSecond) - 16) / 2;
  interrupts();

  return formerSpeed;
}

uint8_t twi_status()
{
  uint8_t rc = TWI_STATUS_BUSY;
  noInterrupts();
  if (TWI_READY == twi_state)
  {
    rc = twi_error2status();
  }
  interrupts();
  return rc;
}

uint8_t twi_getDataFromRead(uint8_t *data, uint8_t length)
{
  uint8_t rc = 0;
  noInterrupts();
  if ((TWI_READY == twi_state) && (twi_error == 0xFF) && (length >= twi_masterBufferIndex))
  {
    memcpy(data, twi_masterBuffer, twi_masterBufferIndex);
    rc = twi_masterBufferIndex;
  }
  interrupts();
  return rc;
}
  
//
// begins an async read operation
//
uint8_t twi_beginReadFrom(uint8_t address, uint8_t* data, uint8_t length)
{
  uint8_t i;

  // ensure data will fit into buffer
  if(TWI_BUFFER_LENGTH < length){
    return TWI_STATUS_BAD_ARGS;
  }
  if (TWI_READY != twi_state) return TWI_STATUS_BUSY;

  noInterrupts();
  twi_state = TWI_MRX;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // initialize buffer iteration vars
  twi_masterBufferIndex = 0;
  twi_masterBufferLength = length-1;  // This is not intuitive, read on...
  // On receive, the previously configured ACK/NACK setting is transmitted in
  // response to the received byte before the interrupt is signalled. 
  // Therefor we must actually set NACK when the _next_ to last byte is
  // received, causing that NACK to be sent in response to receiving the last
  // expected byte of data.

  // build sla+r, slave device address + r bit
  twi_slarw = TW_READ;
  twi_slarw |= address << 1;

  // send start condition
  TWCR = twi_twcr_base | _BV(TWINT) | _BV(TWSTA);
  interrupts();

  return TWI_STATUS_OK;
}

/* 
* Function twi_readFrom
* Desc     attempts to become twi bus master and read a
*          series of bytes from a device on the bus. blocks until data received.
* Input    address: 7bit i2c device address
*          data: pointer to byte array
*          length: number of bytes to read into array
* Output   number of bytes read
*/
uint8_t twi_readFrom(uint8_t address, uint8_t* data, uint8_t length)
{
  uint8_t i;
  uint8_t rc;
  while(TWI_READY != twi_state);
  rc = twi_beginReadFrom(address, data, length);
  if (rc != TWI_STATUS_OK) return 0;

  // wait for read operation to complete
  while(TWI_READY != twi_state);
  return twi_getDataFromRead(data, length);  
}

/* 
* Function twi_readFromAt
* Desc     attempts to become twi bus master and write a data (register or memory) offset
*          to a slave device on the bus, then repeat start and read a
*          series of bytes from the slave at the slave-side offset
*          IOW this is the bus behaviour for reading i2c GPIO or EEPROM chips
* Input    address: 7bit i2c device address
*          offset: slave-side data offset for read
*          data: pointer to byte array
*          length: number of bytes to read into array
* Output   number of bytes read
*/
uint8_t twi_beginReadFromAt(uint8_t address, uint8_t offset, uint8_t* data, uint8_t length)
{
  uint8_t i;

  // ensure data will fit into buffer
  if(TWI_BUFFER_LENGTH < length){
    return TWI_STATUS_BAD_ARGS;
  }
  if (TWI_READY != twi_state) return TWI_STATUS_BUSY;

  noInterrupts();
  twi_state = TWI_MRXR;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // save the slave register offset for later use
  twi_slAtLSB = offset;
  twi_slAtBytes = 1;

  // initialize buffer iteration vars
  twi_masterBufferIndex = 0;
  twi_masterBufferLength = length-1;  // This is not intuitive, read on...
  // On receive, the previously configured ACK/NACK setting is transmitted in
  // response to the received byte before the interrupt is signalled. 
  // Therefor we must actually set NACK when the _next_ to last byte is
  // received, causing that NACK to be sent in response to receiving the last
  // expected byte of data.

  // build sla+w, slave device address + w bit
  // NOT A TYPO - we need to write the register offset as data before sending
  // repeat start and sla+r later to perform the read from the slave-side
  // register offset
  twi_slarw = TW_WRITE;
  twi_slarw |= address << 1;

  // send start condition
  TWCR = twi_twcr_base | _BV(TWINT) | _BV(TWSTA);
  interrupts();

  return TWI_STATUS_OK;
}

uint8_t twi_readFromAt(uint8_t address, uint8_t offset, uint8_t* data, uint8_t length)
{
  uint8_t rc;

  while(TWI_READY != twi_state);

  rc = twi_beginReadFromAt(address, offset, data, length);
  if (rc != TWI_STATUS_OK) return 0;

  // wait for read operation to complete
  while(TWI_READY != twi_state);
  return twi_getDataFromRead(data, length);
}

/* 
* Function twi_readFromAt2
* Desc     attempts to become twi bus master and write a data (register or memory) offset
*          to a slave device on the bus, then repeat start and read a
*          series of bytes from the slave at the slave-side offset
*          IOW this is the bus behaviour for reading i2c GPIO or EEPROM chips
* Input    address: 7bit i2c device address
*          offset: slave-side 16-bit data offset for read
*          data: pointer to byte array
*          length: number of bytes to read into array
* Output   number of bytes read
*/
uint8_t twi_beginReadFromAt2(uint8_t address, uint16_t offset, uint8_t* data, uint8_t length)
{
  uint8_t i;

  // ensure data will fit into buffer
  if(TWI_BUFFER_LENGTH < length){
    return TWI_STATUS_BAD_ARGS;
  }

  // wait until twi is ready, become master receiver
  if (TWI_READY != twi_state) return TWI_STATUS_BUSY;

  noInterrupts();
  twi_state = TWI_MRXR;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // save the slave register offset for later use
  twi_slAtMSB = offset >> 8;
  twi_slAtLSB = offset & 0xFF;
  twi_slAtBytes = 2;

  // initialize buffer iteration vars
  twi_masterBufferIndex = 0;
  twi_masterBufferLength = length-1;  // This is not intuitive, read on...
  // On receive, the previously configured ACK/NACK setting is transmitted in
  // response to the received byte before the interrupt is signalled. 
  // Therefor we must actually set NACK when the _next_ to last byte is
  // received, causing that NACK to be sent in response to receiving the last
  // expected byte of data.

  // build sla+w, slave device address + w bit
  // NOT A TYPO - we need to write the register offset as data before sending
  // repeat start and sla+r later to perform the read from the slave-side
  // register offset
  twi_slarw = TW_WRITE;
  twi_slarw |= address << 1;

  // send start condition
  TWCR = twi_twcr_base | _BV(TWINT) | _BV(TWSTA);
  interrupts();

  return TWI_STATUS_OK;
}

uint8_t twi_readFromAt2(uint8_t address, uint16_t offset, uint8_t* data, uint8_t length)
{
  uint8_t rc;

  // wait for read operation to complete
  while(TWI_READY != twi_state);
  if (twi_error != 0xFF) return 0;
   
  rc = twi_beginReadFromAt2(address, offset, data, length);
  if (rc != TWI_STATUS_OK) return 0;

  while (TWI_READY != twi_state);
  return twi_getDataFromRead(data, length);
}

/* 
* Function twi_writeTo
* Desc     attempts to become twi bus master and write a
*          series of bytes to a device on the bus
* Input    address: 7bit i2c device address
*          data: pointer to byte array
*          length: number of bytes in array
*          wait: boolean indicating to wait for write or not
* Output   0 .. success
*          1 .. length to long for buffer
*          2 .. address send, NACK received
*          3 .. data send, NACK received
*          4 .. other twi error (lost bus arbitration, bus error, ..)
*/
uint8_t twi_beginWriteTo(uint8_t address, uint8_t* data, uint8_t length)
{
  uint8_t i;

  // ensure data will fit into buffer
  if(TWI_BUFFER_LENGTH < length){
    return TWI_STATUS_BAD_ARGS;
  }
  if (TWI_READY != twi_state) return TWI_STATUS_BUSY;
  
  noInterrupts();
  twi_state = TWI_MTX;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // initialize buffer iteration vars
  twi_masterBufferIndex = 0;
  twi_masterBufferLength = length;

  // copy data to twi buffer
  for(i = 0; i < length; ++i){
    twi_masterBuffer[i] = data[i];
  }

  // build sla+w, slave device address + w bit
  twi_slarw = TW_WRITE;
  twi_slarw |= address << 1;

  // send start condition
  TWCR = twi_twcr_base | _BV(TWINT) | _BV(TWSTA);
  interrupts();

  return TWI_STATUS_OK;
}

uint8_t twi_writeTo(uint8_t address, uint8_t* data, uint8_t length)
{
  uint8_t rc;

  while (TWI_READY != twi_state);

  rc =  twi_beginWriteTo(address, data, length);
  if (rc != TWI_STATUS_OK) return rc;

  while (TWI_READY != twi_state);

  return twi_error2status();
}

/* 
* Function twi_transmit
* Desc     fills slave tx buffer with data
*          must be called in slave tx event callback
* Input    data: pointer to byte array
*          length: number of bytes in array
* Output   1 length too long for buffer
*          2 not slave transmitter
*          0 ok
* NOTE: This function relies upon being called by twi_onSlaveTransmit handler,
*       itself called within the TWI interrupt handler such that the buffer
*       contents provided here will subsequently be transmitted.
*/
uint8_t twi_transmit(uint8_t* data, uint8_t length)
{
  uint8_t i;

  // ensure data will fit into buffer
  if(TWI_BUFFER_LENGTH < length){
    return 1;
  }

  noInterrupts();
  // ensure we are currently a slave transmitter
  if(TWI_STX != twi_state){
    return 2;
  }

  // set length and copy data into tx buffer
  twi_txBufferLength = length;
  for(i = 0; i < length; ++i){ // TODO: use memcpy here
    twi_txBuffer[i] = data[i];
  }
  interrupts();

  return 0;
}

/* 
* Function twi_attachSlaveRxEvent
* Desc     sets function called before a slave read operation
* Input    function: callback function to use
* Output   none
*/
void twi_attachSlaveRxEvent( void (*function)(uint8_t*, int) )
{
  twi_onSlaveReceive = function;
}

/* 
* Function twi_attachSlaveTxEvent
* Desc     sets function called before a slave write operation
* Input    function: callback function to use
* Output   none
*/
void twi_attachSlaveTxEvent( void (*function)(void) )
{
  twi_onSlaveTransmit = function;
}

/* 
* Function twi_reply
* Desc     Write TWCR with TWINT bit, thus acknowledging it, which causes
*          the next logical TW operation to occur.  If TW is in MT or ST
*          mode and a byte loaded in TWDR it is transmitted.  Otherwise only
*          the ACK/NACK is sent per the 'ack' input.  The TWEN and TWIE bits
*          are on merely to keep them enabled.
* Input    ack: byte indicating to ack or to nack
* Output   none
*/
void twi_reply(uint8_t ack)
{
  // transmit master read ready signal, with or without ack
  if(ack){
    TWCR = twi_twcr_base | _BV(TWINT) | _BV(TWEA);
  }else{
    TWCR = (twi_twcr_base | _BV(TWINT)) & ~_BV(TWEA);
  }
}
//
// converts the twi_error value into an externally defined status code
//
uint8_t twi_error2status()
{
  int err = twi_error;

  if (err == 0xFF)
    return TWI_STATUS_OK;	// success
  else if (err == TW_MT_SLA_NACK)
    return TWI_STATUS_SLA_NACK;	// error: address send, nack received
  else if (err == TW_MT_DATA_NACK)
    return TWI_STATUS_DATA_NACK;	// error: data send, nack received
  else
    return TWI_STATUS_OTHER;	// other twi error}
}

/* 
* Function twi_stop
* Desc     relinquishes bus master status
* Input    none
* Output   none
*/
void twi_stop(void)
{
  // send stop condition
  TWCR = twi_twcr_base | _BV(TWINT) | _BV(TWSTO);

  // wait for stop condition to be executed on bus
  // TWINT is not set after a stop condition!
  while(TWCR & _BV(TWSTO)){
    continue;
  }

  // update twi state
  twi_state = TWI_READY;  
}

/* 
* Function twi_releaseBus
* Desc     releases bus control
* Input    none
* Output   none
*/
void twi_releaseBus(void)
{
  noInterrupts();
  // release bus by acknowledging TWINT bit.
  TWCR = twi_twcr_base | _BV(TWINT);

  // update twi state
  twi_state = TWI_READY;
  interrupts();
}

ISR(TWI_vect)
{
  switch(TW_STATUS){   // TWSR with prescaler bits masked
    // All Master
    case TW_START:     // sent start condition
    case TW_REP_START: // sent repeated start condition
      // copy device address and r/w bit to output register and ack
      TWDR = twi_slarw;
      twi_reply(1);
      break;

      // Master Transmitter
    case TW_MT_SLA_ACK:  // slave receiver acked address
      if (twi_state == TWI_MRXR){
        // tell the slave what register offset to use for subsequent read
        if (twi_slAtBytes-- ==  2) {
          TWDR = twi_slAtMSB;
        } else {
          TWDR = twi_slAtLSB;
        }
        twi_reply(1);
        break;
      }
    case TW_MT_DATA_ACK: // slave receiver acked data
      if (twi_state == TWI_MRXR){
        if (twi_slAtBytes-- ==  1) {
          TWDR = twi_slAtLSB;
          twi_reply(1);
        } else {
          twi_slarw |= TW_READ;  // Configure subsequent operation to be a read
          // send a repeat start
          TWCR = twi_twcr_base | _BV(TWINT) | _BV(TWSTA);
        }
        break;
      }
      // if there is data to send, send it, otherwise stop 
      if(twi_masterBufferIndex < twi_masterBufferLength){
        // copy data to output register and ack
        TWDR = twi_masterBuffer[twi_masterBufferIndex++];
        twi_reply(1);
      }else{
        twi_stop();
      }
      break;
    case TW_MT_SLA_NACK:  // address sent, nack received
      twi_error = TW_MT_SLA_NACK;
      twi_stop();
      break;
    case TW_MT_DATA_NACK: // data sent, nack received
      twi_error = TW_MT_DATA_NACK;
      twi_stop();
      break;
    case TW_MT_ARB_LOST: // lost bus arbitration
      twi_error = TW_MT_ARB_LOST;
      twi_releaseBus();
      break;

      // Master Receiver
    case TW_MR_DATA_ACK: // data received, ack sent
      // put byte into buffer
      twi_masterBuffer[twi_masterBufferIndex++] = TWDR;
    case TW_MR_SLA_ACK:  // address sent, ack received
      // ack if more bytes are expected, otherwise nack
      if(twi_masterBufferIndex < twi_masterBufferLength){
        twi_reply(true);
      }else{
        twi_reply(false);
      }
      break;
    case TW_MR_DATA_NACK: // data received, nack sent
      // put final byte into buffer
      twi_masterBuffer[twi_masterBufferIndex++] = TWDR;
    case TW_MR_SLA_NACK: // address sent, nack received
      twi_stop();
      break;
      // TW_MR_ARB_LOST handled by TW_MT_ARB_LOST case

      // Slave Receiver
    case TW_SR_SLA_ACK:   // addressed, returned ack
    case TW_SR_GCALL_ACK: // addressed generally, returned ack
    case TW_SR_ARB_LOST_SLA_ACK:   // lost arbitration, returned ack
    case TW_SR_ARB_LOST_GCALL_ACK: // lost arbitration, returned ack
      // enter slave receiver mode
      twi_state = TWI_SRX;
      // indicate that rx buffer can be overwritten and ack
      twi_rxBufferIndex = 0;
      twi_reply(true);
      break;
    case TW_SR_DATA_ACK:       // data received, returned ack
    case TW_SR_GCALL_DATA_ACK: // data received generally, returned ack
      // if there is still room in the rx buffer
      if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){
        // put byte in buffer and ack
        twi_rxBuffer[twi_rxBufferIndex++] = TWDR;
        twi_reply(true);
      }else{
        // otherwise nack
        twi_reply(false);
      }
      break;
    case TW_SR_STOP: // stop or repeated start condition received
      // put a null char after data if there's room
      if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){
        twi_rxBuffer[twi_rxBufferIndex] = '\0';
      }
      // callback to user defined callback
      twi_onSlaveReceive(twi_rxBuffer, twi_rxBufferIndex);
      // ack future responses
      twi_reply(true);
      // leave slave receiver state
      twi_state = TWI_READY;
      break;
    case TW_SR_DATA_NACK:       // data received, returned nack
    case TW_SR_GCALL_DATA_NACK: // data received generally, returned nack
      // nack back at master
      twi_reply(false);
      break;

      // Slave Transmitter
    case TW_ST_SLA_ACK:          // addressed, returned ack
    case TW_ST_ARB_LOST_SLA_ACK: // arbitration lost, returned ack
      // enter slave transmitter mode
      twi_state = TWI_STX;
      // ready the tx buffer index for iteration
      twi_txBufferIndex = 0;
      // set tx buffer length to be zero, to verify if user changes it
      twi_txBufferLength = 0;
      // request for txBuffer to be filled and length to be set
      // note: user must call twi_transmit(bytes, length) to do this
      twi_onSlaveTransmit();
      // if they didn't change buffer & length, initialize it
      if(0 == twi_txBufferLength){
        twi_txBufferLength = 1;
        twi_txBuffer[0] = 0x00;
      }
      // transmit first byte from buffer, fall
    case TW_ST_DATA_ACK: // byte sent, ack returned
      // copy data to output register
      TWDR = twi_txBuffer[twi_txBufferIndex++];
      // if there is more to send, ack, otherwise nack
      if(twi_txBufferIndex < twi_txBufferLength){
        twi_reply(true);
      }else{
        twi_reply(false);
      }
      break;
    case TW_ST_DATA_NACK: // received nack, we are done 
    case TW_ST_LAST_DATA: // received ack, but we are done already!
      // ack future responses
      twi_reply(true);
      // leave slave receiver state
      twi_state = TWI_READY;
      break;

      // All
    case TW_NO_INFO:   // no state information
      break;
    case TW_BUS_ERROR: // bus error, illegal stop/start
      twi_error = TW_BUS_ERROR;
      twi_stop();
      break;
  }
}

