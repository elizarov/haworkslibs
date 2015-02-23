
#include "TWIMaster.h"

#include <avr/io.h>
#include <util/twi.h>

//#define TWI_DEBUG // writes to Serial at every stage to debug I2C bus problems

const long TIMEOUT = 200; // 200ms timeout

const uint8_t PIN_BITS = _BV(PORTC4) | _BV(PORTC5);

// Singleton instance 
TWIMasterClass TWIMaster; 

TWIMasterClass::TWIMasterClass(twi_speed_t speed) {
  // configure twi pins as input and pullup
  DDRC &= ~PIN_BITS; // clear is input (just in case...)
  PORTC |= PIN_BITS; // set to pullup
  setSpeed(speed);
}

inline void TWIMasterClass::setSpeed(twi_speed_t speed) {
  cli();
  TWBR = (uint8_t)speed;
  TWSR = (uint8_t)(speed >> 8);
  sei();
}

inline void TWIMasterClass::restore() {
  // restore TWEN, TWIE, TWEA bits, clear interrupt flag
  TWCR = _BV(TWINT) | _twcr;
  _active = false;
#ifdef TWI_DEBUG
  Serial.println(']');
#endif
}

inline void TWIMasterClass::abort() {
  TWCR = 0; // abort by turning TWI off
  restore();
}

// result != 0 means error, error code is returned
uint8_t TWIMasterClass::wait(uint8_t expected) {
  Timeout timeout(TIMEOUT);
  while ((TWCR & _BV(TWINT)) == 0) // spin wait until timeout
    if (timeout.check()) {
#ifdef TWI_DEBUG
      Serial.print(F("=TIMEOUT"));
#endif
      abort();
      return 0xfe; // timeout error code
    }
  uint8_t status = TW_STATUS;
  if (status != expected) {
#ifdef TWI_DEBUG
    Serial.print('=');
    Serial.print(status, HEX);
#endif
    switch (status) {
    case TW_MT_ARB_LOST: // same as TW_MR_ARB_LOST
      restore(); // no need to abort -- gracefully return to whatever mode TWI was in
      break;
    case TW_MT_SLA_NACK: // will need to send stop and release bus on all of these
    case TW_MT_DATA_NACK:
    case TW_MR_SLA_NACK:
    case TW_BUS_ERROR: // stop recovers from bus errors, too
      stop(); 
      break;
    default: // otherwise abort (something went horribly wrong)
      abort();
    }
    return status == 0 ? 0xff : status; // replace TWI status 0 with erorr code 0xff
  }
  return 0;
}

// result != 0 means error, error code is returned
uint8_t TWIMasterClass::start() {
  bool repStart = _active;
#ifdef TWI_DEBUG
  Serial.print(repStart ? '!' : '[');
#endif
  if (!repStart) {
    // on first start save original TWEN, TWIE, and TWEA bits register value
    cli();
    _twcr = TWCR & (_BV(TWEN) | _BV(TWIE) | _BV(TWEA)); 
    if (_twcr & _BV(TWEN)) 
      TWCR = 0; // something was going on (because TWI was enabled) -- terminate it to reset TWI state machine
    sei();
    if (_twcr & _BV(TWIE)) // if interrupts were enabled, we assume that TWI slave is also working here
      _twcr |= _BV(TWEA); // need to make sure to enable ack in restore() when we are done
    _active = true;
  }
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); // enable & start, no interrups
  uint8_t status = wait(repStart ? TW_REP_START : TW_START);
  if (status != 0)
    return status;
#ifdef TWI_DEBUG
  Serial.print('+');
#endif
  return 0;
}

void TWIMasterClass::stop() {
  if (!_active)
    return;
#ifdef TWI_DEBUG
  Serial.print('-');
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
      abort();
      return;
    }
  // all Ok -- restore original state
  restore();
#ifdef TWI_DEBUG
  Serial.print(']');
#endif
}

// result != 0 means error, error code is returned
uint8_t TWIMasterClass::transmit(uint8_t addr, const void *tx, uint8_t n, bool keepBus) {
  uint8_t status = start();
  if (status != 0)
    return status;
#ifdef TWI_DEBUG
  Serial.print('T');
#endif
  // write slave address + W, then data bits
  uint8_t b = (addr << 1) | TW_WRITE;
  uint8_t expect = TW_MT_SLA_ACK;
  uint8_t *ptr = (uint8_t*)tx;
  while (true) {
    TWDR = b;
    TWCR = _BV(TWINT) | _BV(TWEN);
    status = wait(expect);
    if (status != 0)
      return status;
    if (n-- == 0)
      break;
    b = *(ptr++);
    expect = TW_MT_DATA_ACK;
  }
  if (!keepBus)
    stop();
  return 0;
}

// result != 0 means error, error code is returned
uint8_t TWIMasterClass::receive(uint8_t addr, void *rx, uint8_t n, bool keepBus) {
  uint8_t status = start();
  if (status != 0)
    return status;
#ifdef TWI_DEBUG
  Serial.print('R');
#endif
  // write slave address
  TWDR = (addr << 1) | TW_READ;
  TWCR = _BV(TWINT) | _BV(TWEN);
  status = wait(TW_MR_SLA_ACK);
  if (status != 0)
    return status;
  // read data bits
  uint8_t *ptr = (uint8_t*)rx;
  while (n-- != 0) {
    uint8_t ack = n == 0 ? 0 : _BV(TWEA); // ack only last received byte
    TWCR = _BV(TWINT) | _BV(TWEN) | ack; 
    status = wait(ack ? TW_MR_DATA_ACK : TW_MR_DATA_NACK);
    if (status != 0)
      return status;
    *(ptr++) = TWDR;
  }
  if (!keepBus)
    stop();
  return 0;
}
