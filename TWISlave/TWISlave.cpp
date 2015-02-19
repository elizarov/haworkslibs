
#include <TWISlave.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

//#define TWI_DEBUG // writes to Serial at every stage to debug I2C bus problems

TWISlaveClass TWISlave;

void TWISlaveClass::begin(uint8_t addr) {
  cli();
  TWAR = addr << 1;
  TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
  sei();
}

void TWISlaveClass::use(void *buf, uint8_t size) {
  _buf = (uint8_t*)buf;
  _rem_size = size;
  _done_size = 0;
}

inline void TWISlaveClass::nackIfDone() {
  if (_rem_size == 0)
    TWCR &= ~_BV(TWEA);
}

void TWISlaveClass::receiveByte() {
  if (_rem_size > 0) {
#ifdef TWI_DEBUG
    Serial.print('R');
    Serial.print(TWDR, HEX);
#endif
    *(_buf++) = TWDR;
    _rem_size--;
    _done_size++;
  }
}

void TWISlaveClass::transmitByte() {
  if (_rem_size > 0) {
#ifdef TWI_DEBUG
    Serial.print('T');
    Serial.print(*_buf, HEX);
#endif
    TWDR = *(_buf++);
    _rem_size--;
    _done_size++;
  } else
    TWDR = 0; // trasmit zero byte to master if we have nothing to send
  nackIfDone();
}

inline void TWISlaveClass::_processInterrupt() {
#ifdef TWI_DEBUG
  Serial.print('S');
  Serial.print(TW_STATUS, HEX);
#endif
  switch (TW_STATUS) {
  // Slave recive states
  case TW_SR_SLA_ACK:
    twiSlaveCall(TWDR >> 1, false);
    nackIfDone();
    break; 
  case TW_SR_DATA_ACK:
    receiveByte();
    nackIfDone();
    break;
  case TW_SR_DATA_NACK: // done receive
    receiveByte();
    twiSlaveDone(_done_size, false);
    TWCR |= _BV(TWEA);
    break;
  case TW_SR_STOP:
#ifdef TWI_DEBUG
    Serial.println('.');
#endif
    twiSlaveDone(_done_size, false);
    TWCR |= _BV(TWEA);
    break;
  // Slave transmit states
  case TW_ST_SLA_ACK:
    twiSlaveCall(TWDR >> 1, true);
    transmitByte();
    break;
  case TW_ST_DATA_ACK:
    transmitByte();
    break;
  case TW_ST_DATA_NACK: // done transmit
  case TW_ST_LAST_DATA:
    twiSlaveDone(_done_size, true);
    TWCR |= _BV(TWEA);
    break;
  case TW_BUS_ERROR:
#ifdef TWI_DEBUG
    Serial.println('!');
#endif
    TWCR |= _BV(TWEA) | _BV(TWSTO); // recover from bus error
    break;
  default: // something else went wrong... should not happen, but enable acknowledge
#ifdef TWI_DEBUG
    Serial.println('?');
#endif
    TWCR |= _BV(TWEA);
  }
  TWCR |= _BV(TWINT);
}

ISR(TWI_vect) {
  TWISlave._processInterrupt();
}
