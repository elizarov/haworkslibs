
#include "TWIMaster.h"

#include <avr/io.h>
#include <util/twi.h>

//#define TWI_DEBUG // writes to Serial at every stage to debug I2C bus problems

const unsigned long TIMEOUT = 200; // 200ms timeout

const uint8_t PIN_BITS = _BV(PORTC4) | _BV(PORTC5);

// result != 0 means error, error code is returned
static uint8_t twiWait(uint8_t expected) {
  Timeout timeout(TIMEOUT);
  while ((TWCR & _BV(TWINT)) == 0) // spin wait until timeout
    if (timeout.check()) {
#ifdef TWI_DEBUG
      Serial.print(F("=TIMEOUT"));
#endif
      return 0xfe; // timeout error code
    }
  uint8_t status = TW_STATUS;
  if (status != expected) {
#ifdef TWI_DEBUG
    Serial.print('=');
    Serial.print(status, HEX);
#endif
    return status == 0 ? 0xff : status; // replace TWI status 0 with erorr code 0xff
  }
  return 0;
}

TWIMaster::TWIMaster(twi_speed_t speed) {
#ifdef TWI_DEBUG
  Serial.print('{');
#endif
  cli();
  // configure twi pins as input and pullup
  DDRC &= ~PIN_BITS; // clear is input
  PORTC |= PIN_BITS; // set to pullup
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
#ifdef TWI_DEBUG
  Serial.print(':');
#endif
}

TWIMaster::~TWIMaster() {
#ifdef TWI_DEBUG
  Serial.print('~');
#endif
  stop();
  cli();
  // restore baud reate
  TWBR = _twbr;
  TWSR = _twsr;
  // restore TWEN, TWIE, TWEA bits, clear interrupt flag
  TWCR = _BV(TWINT) | (_twcr & (_BV(TWEN) | _BV(TWIE) | _BV(TWEA)));
  sei();
#ifdef TWI_DEBUG
  Serial.println('}');
#endif
}

// result != 0 means error, error code is returned
uint8_t TWIMaster::start() {
#ifdef TWI_DEBUG
  Serial.print('+');
#endif
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
  uint8_t status = twiWait(_active ? TW_REP_START : TW_START);
  if (status != 0)
    return status;
  _active = true;
#ifdef TWI_DEBUG
  Serial.print('[');
#endif
  return 0;
}

void TWIMaster::stop() {
  if (!_active)
    return;
#ifdef TWI_DEBUG
  Serial.print(']');
#endif
  // send stop 
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
  // wait until stop finished
  Timeout timeout(TIMEOUT);
  while (TWCR & _BV(TWSTO)) // spin wait until timeout
    if (timeout.check()) {
#ifdef TWI_DEBUG
      Serial.print(F("=TIMEOUT"));
#endif
      TWCR = 0; // abort by turning TWI off
      break;
    }
  _active = false; 
#ifdef TWI_DEBUG
  Serial.print('-');
#endif
}

// result != 0 means error, error code is returned
uint8_t TWIMaster::transmit(uint8_t addr, uint8_t* buf, uint8_t n) {
  uint8_t status = start();
  if (status != 0)
    return status;
#ifdef TWI_DEBUG
  Serial.print('T');
#endif
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
#ifdef TWI_DEBUG
  Serial.print('R');
#endif
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
