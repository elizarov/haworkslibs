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
TwoWire.cpp - TWI/I2C library for Wiring & Arduino
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

extern "C" {
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "extended_twi.h"
}

#include "ExtendedWire.h"

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t* ExtendedWire::rxBuffer = 0;
uint8_t ExtendedWire::rxBufferIndex = 0;
uint8_t ExtendedWire::rxBufferLength = 0;
uint8_t ExtendedWire::asyncRxActive = 0;

uint8_t ExtendedWire::txAddress = 0;
uint8_t* ExtendedWire::txBuffer = 0;
uint8_t ExtendedWire::txBufferIndex = 0;
uint8_t ExtendedWire::txBufferLength = 0;

uint8_t ExtendedWire::transmitting = 0;
void (*ExtendedWire::user_onRequest)(void);
void (*ExtendedWire::user_onReceive)(int);

// Constructors ////////////////////////////////////////////////////////////////

ExtendedWire::ExtendedWire()
{
}

// Public Methods //////////////////////////////////////////////////////////////

void ExtendedWire::begin()
{
  // init buffer for reads
  rxBuffer = (uint8_t*) calloc(BUFFER_LENGTH, sizeof(uint8_t));
  rxBufferIndex = 0;
  rxBufferLength = 0;

  // init buffer for writes
  txBuffer = (uint8_t*) calloc(BUFFER_LENGTH, sizeof(uint8_t));
  txBufferIndex = 0;
  txBufferLength = 0;

  twi_init();
}

void ExtendedWire::begin(uint8_t address)
{
  twi_setAddress(address);
  twi_attachSlaveTxEvent(onRequestService);
  twi_attachSlaveRxEvent(onReceiveService);
  begin();
}

void ExtendedWire::begin(int address)
{
  begin((uint8_t)address);
}

void ExtendedWire::enablePullupResistors(uint8_t enable)
{
  twi_enable_pullup_resistors(enable);
}

long unsigned int ExtendedWire::setSpeed(long unsigned int bitsPerSecond)
{
  return twi_setSpeed(bitsPerSecond);
}

uint8_t ExtendedWire::status()
{
  // note: the codes in the two .h files (twi.h and Wire.h) have been
  // chosen to match so no translation is necessary between the return 
  // value of twi_operationComplete() and this function.
  // be sure to maintain this matching if codes are changed.
  return twi_status();
}


//
// *** SYNCHRONOUS READ FUNCTIONS ***
//

uint8_t ExtendedWire::requestFrom(uint8_t address, uint8_t quantity)
{
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }

  // perform read into buffer
  uint8_t read = twi_readFrom(address, rxBuffer, quantity);

  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = read;

  return read;
}

uint8_t ExtendedWire::requestFrom(int address, int quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity);
}

uint8_t ExtendedWire::requestFromAt(uint8_t address, uint8_t offset, uint8_t quantity)
{
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  // perform read into buffer
  uint8_t read = twi_readFromAt(address, offset, rxBuffer, quantity);

  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = read;

  return read;
}

uint8_t ExtendedWire::requestFromAt(int address, int offset, int quantity)
{
  return requestFromAt((uint8_t)address, (uint8_t)offset, (uint8_t)quantity);
}

uint8_t ExtendedWire::requestFromAt2(uint8_t address, int offset, uint8_t quantity)
{
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  // perform read into buffer
  uint8_t read = twi_readFromAt2(address, offset, rxBuffer, quantity);

  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = read;

  return read;
}

uint8_t ExtendedWire::requestFromAt2(int address, int offset, int quantity)
{
  return requestFromAt2((uint8_t)address, offset, (uint8_t)quantity);
}

//
// *** ASYNCHRONOUS READ FUNCTIONS ***
//


uint8_t ExtendedWire::kickoffRequestFrom(uint8_t address, uint8_t quantity)
{
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  
  uint8_t rc = twi_beginReadFrom(address, rxBuffer, quantity);
  asyncRxActive = rc == TWI_STATUS_OK;
  return rc;
}

uint8_t ExtendedWire::kickoffRequestFrom(int address, int quantity)
{
  return kickoffRequestFrom((uint8_t)address, (uint8_t)quantity);
}

uint8_t ExtendedWire::kickoffRequestFromAt(uint8_t address, uint8_t offset, uint8_t quantity)
{
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  // perform read into buffer
  uint8_t rc = twi_beginReadFromAt(address, offset, rxBuffer, quantity);
  asyncRxActive = (rc == TWI_STATUS_OK);
  // Serial.print("Kickoff from at "); Serial.println((int)asyncRxActive);
  return rc;
}

uint8_t ExtendedWire::kickoffRequestFromAt(int address, int offset, int quantity)
{
  return kickoffRequestFromAt((uint8_t)address, (uint8_t)offset, (uint8_t)quantity);
}

uint8_t ExtendedWire::kickoffRequestFromAt2(uint8_t address, int offset, uint8_t quantity)
{
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  // perform read into buffer
  uint8_t rc = twi_beginReadFromAt2(address, offset, rxBuffer, quantity);
  asyncRxActive = rc == TWI_STATUS_OK;
  return rc;
}

uint8_t ExtendedWire::kickoffRequestFromAt2(int address, int offset, int quantity)
{
  return kickoffRequestFromAt2((uint8_t)address, offset, (uint8_t)quantity);
}

//
// *** TRANSMIT FUNCTIONS ***
//

void ExtendedWire::beginTransmission(uint8_t address)
{
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
}

void ExtendedWire::beginTransmission(int address)
{
  beginTransmission((uint8_t)address);
}

void ExtendedWire::beginTransmissionAt(uint8_t address, uint8_t offset)
{
  beginTransmission(address);
  send(offset);
}

void ExtendedWire::beginTransmissionAt(int address, int offset)
{
  beginTransmissionAt((uint8_t)address, (uint8_t)offset);
}

void ExtendedWire::beginTransmissionAt2(uint8_t address, int offset)
{
  beginTransmission((uint8_t)address);
  send((uint8_t)(offset >> 8));    // MSB of address transmitted first
  send((uint8_t)(offset & 0xFF));  // LSB of address transmitted second
}

void ExtendedWire::beginTransmissionAt2(int address, int offset)
{
  beginTransmissionAt2((uint8_t)address, offset);
}

uint8_t ExtendedWire::kickoffTransmission(void)
{
  uint8_t ret = twi_beginWriteTo(txAddress, txBuffer, txBufferLength);

  if (ret != TWI_STATUS_BUSY)
  {
    // reset tx buffer iterator vars
    txBufferIndex = 0;
    txBufferLength = 0;
    // indicate that we are done transmitting
    transmitting = 0;
  }

  return ret;
}

uint8_t ExtendedWire::endTransmission()
{
  // transmit buffer (blocking)
  uint8_t ret = twi_writeTo(txAddress, txBuffer, txBufferLength);

  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  // indicate that we are done transmitting
  transmitting = 0;
  return ret;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
void ExtendedWire::send(uint8_t data)
{
  if(transmitting){
    // in master transmitter mode
    // don't bother if buffer is full
    if(txBufferLength >= BUFFER_LENGTH){
      return;
    }
    // put byte in tx buffer
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    // update amount in buffer   
    txBufferLength = txBufferIndex;
  }else{
    // in slave send mode
    // reply to master
    twi_transmit(&data, 1);
  }
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
void ExtendedWire::send(uint8_t* data, uint8_t quantity)
{
  if(transmitting){
    // in master transmitter mode
    for(uint8_t i = 0; i < quantity; ++i){
      send(data[i]);
    }
  }else{
    // in slave send mode
    // reply to master
    twi_transmit(data, quantity);
  }
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
void ExtendedWire::send(char* data)
{
  send((uint8_t*)data, strlen(data));
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
void ExtendedWire::send(int data)
{
  send((uint8_t)data);
}

void ExtendedWire::asyncBufferUpdate()
{
  if (asyncRxActive)
  {
    // make sure the read is complete before messing around with
    // buffer pointers
    if (twi_status() == TWI_STATUS_BUSY) return;
    // get a copy of the received data and init buffer pointer
    // if there was an error, we'll get a data length of zero returned.
    rxBufferLength = twi_getDataFromRead(rxBuffer, BUFFER_LENGTH);
    rxBufferIndex = 0;
    asyncRxActive = false;
  }
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
uint8_t ExtendedWire::available(void)
{
  asyncBufferUpdate();
  return rxBufferLength - rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after any requestFrom...() variant
uint8_t ExtendedWire::receive(void)
{
  asyncBufferUpdate();
  // default to returning null char
  // for people using with char strings
  uint8_t value = '\0';

  // get each successive byte on each call
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
    ++rxBufferIndex;
  }

  return value;
}

// behind the scenes function that is called when data is received
void ExtendedWire::onReceiveService(uint8_t* inBytes, int numBytes)
{
  // don't bother if user hasn't registered a callback
  if(!user_onReceive){
    return;
  }
  // don't bother if rx buffer is in use by a master requestFrom() op
  // i know this drops data, but it allows for slight stupidity
  // meaning, they may not have read all the master requestFrom() data yet
  if(rxBufferIndex < rxBufferLength){
    return;
  }
  // copy twi rx buffer into local read buffer
  // this enables new reads to happen in parallel
  for(uint8_t i = 0; i < numBytes; ++i){
    rxBuffer[i] = inBytes[i];    
  }
  // set rx iterator vars
  rxBufferIndex = 0;
  rxBufferLength = numBytes;
  // alert user program
  user_onReceive(numBytes);
}

// behind the scenes function that is called when data is requested
void ExtendedWire::onRequestService(void)
{
  // don't bother if user hasn't registered a callback
  if(!user_onRequest){
    return;
  }
  // reset tx buffer iterator vars
  // !!! this will kill any pending pre-master sendTo() activity
  txBufferIndex = 0;
  txBufferLength = 0;
  // alert user program
  user_onRequest();
}

// sets function called on slave write
void ExtendedWire::onReceive( void (*function)(int) )
{
  user_onReceive = function;
}

// sets function called on slave read
void ExtendedWire::onRequest( void (*function)(void) )
{
  user_onRequest = function;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

ExtendedWire Wire = ExtendedWire();

