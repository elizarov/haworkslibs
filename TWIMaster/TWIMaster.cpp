
#include "TWIMaster.h"

#include <avr/io.h>
#include <avr/sleep.h>
#include <util/twi.h>

#include <Arduino.h>

const uint8_t PIN_BITS = _BV(PORTC4) | _BV(PORTC5);

// result != 0 means error, error code is returned
static uint8_t twiWait(uint8_t expected) {
  while ((TWCR & _BV(TWINT)) == 0)
    sleep_mode();
  uint8_t status = TW_STATUS;
  if (status != expected)
    return status == 0 ? 0xff : status; // replace TWI status 0 with erorr code 0xff
  return 0;
}

TWIMaster::TWIMaster(twi_speed_t speed) {
  cli();
  // configure twi pins as input and pullup
  DDRC &= ~PIN_BITS; // clear is input
  PORTC |= PIN_BITS; // set to pullup
  _prr = PRR;
  PRR &= ~_BV(PRTWI); // power on twi if it was off
  // save original TWI register values
  _twcr = TWCR;
  _twbr = TWBR;
  _twsr = TWSR;
  // init TWI register values
  TWCR = 0; // turn off TWI, terminte everything that was going on
  TWBR = (uint8_t)speed;
  TWSR = (uint8_t)(speed >> 8);
  sei();
  _active = false;
}

TWIMaster::~TWIMaster() {
  stop();
  cli();
  // restore baud reate
  TWBR = _twbr;
  TWSR = _twsr;
  // restore TWEN, TWIE, TWEA bits
  TWCR = _BV(TWINT) | (_twcr & (_BV(TWEN) | _BV(TWIE) | _BV(TWEA)));
  // power off twi if it was off
  if (_prr & _BV(PRTWI))
    PRR |= _BV(PRTWI); 
  sei();
}

// result != 0 means error, error code is returned
uint8_t TWIMaster::start() {
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
  uint8_t status = twiWait(_active ? TW_REP_START : TW_START);
  if (status != 0)
    return status;
  _active = true;
  return 0;
}

void TWIMaster::stop() {
  if (!_active)
    return;
  // send stop 
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
  // wait until stop finished
  while (TWCR & _BV(TWSTO))
    sleep_mode();
  _active = false; 
}

// result != 0 means error, error code is returned
uint8_t TWIMaster::transmit(uint8_t addr, uint8_t* buf, uint8_t n) {
  uint8_t status = start();
  if (status != 0)
    return status;
  // write slave address + W, then data bits
  uint8_t b = (addr << 1) | TW_WRITE;
  while (true) {
    TWDR = b;
    TWCR = _BV(TWINT) | _BV(TWEN);
    status = twiWait(TW_MT_SLA_ACK);
    if (status != 0)
      return status;
    if (n-- == 0)
      break;
    b = *(buf++);
  }
  return 0;
}

// result != 0 means error, error code is returned
uint8_t TWIMaster::receive(uint8_t addr, uint8_t* buf, uint8_t n) {
  uint8_t status = start();
  if (status != 0)
    return status;
  // write slave address
  TWDR = (addr << 1) | TW_READ;
  TWCR = _BV(TWINT) | _BV(TWEN);
  status = twiWait(TW_MR_SLA_ACK);
  if (status != 0)
    return status;
  // read data bits
  while (n-- != 0) {
    uint8_t ack = n == 0 ? 0 : _BV(TWEA); // ack only last received byte
    TWCR = _BV(TWINT) | _BV(TWEN) | ack; 
    status = twiWait(ack ? TW_MR_DATA_ACK : TW_MR_DATA_NACK);
    if (status != 0)
      return status;
    *(buf++) = TWDR;
  }
  return 0;
}
