
#include <TWISlave.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

//#define TWI_DEBUG // writes to Serial at every stage to debug I2C bus problems

#ifdef TWI_DEBUG
#include <Arduino.h>
#endif

TWISlaveClass TWISlave;

const uint8_t MAX_DONE_SIZE = 0xff;

void TWISlaveClass::begin(uint8_t addr) {
  cli();
  TWAR = addr << 1;
  TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
  sei();
}

void TWISlaveClass::start() {
  _done_size = 0;
  _rem_size = 0;
  _more = true;
}

void TWISlaveClass::use(void *buf, uint8_t size, bool more) {
  _buf = (uint8_t*)buf;
  _rem_size = size;
  _more = more;
}

inline void TWISlaveClass::nackIfDone() {
  if (_rem_size == 0 || _done_size == MAX_DONE_SIZE)
    TWCR &= ~_BV(TWEA);
}

void TWISlaveClass::receiveByte() {
  if (_rem_size > 0 && _done_size < MAX_DONE_SIZE) {
#ifdef TWI_DEBUG
    Serial.print('R');
    Serial.print(TWDR >> 4, HEX);
    Serial.print(TWDR & 0xf, HEX);
    Serial.print(' ');
#endif
    *(_buf++) = TWDR;
    _rem_size--;
    _done_size++;
  }
}

void TWISlaveClass::transmitByte() {
  if (_rem_size > 0 && _done_size < MAX_DONE_SIZE) {
#ifdef TWI_DEBUG
    Serial.print('T');
    Serial.print(*_buf >> 4, HEX);
    Serial.print(*_buf & 0xf, HEX);
    Serial.print(' ');
#endif
    TWDR = *(_buf++);
    _rem_size--;
    _done_size++;
  } else
    TWDR = 0; // trasmit zero byte to master if we have nothing to send
}

inline void TWISlaveClass::_processInterrupt() {
#ifdef TWI_DEBUG
  Serial.print('S');
  Serial.print(TW_STATUS >> 4, HEX);
  Serial.print(TW_STATUS & 0xf, HEX);
  Serial.print(' ');
#endif
  switch (TW_STATUS) {
  // Slave recive states
  case TW_SR_SLA_ACK:
    start();
    twiSlaveReceive(0, true); // request initial buffer space to receive
    nackIfDone();
    break; 
  case TW_SR_DATA_ACK:
    receiveByte();
    if (_rem_size == 0 && _more)
      twiSlaveReceive(_done_size, true); // request more buffer space to receive
    nackIfDone();
    break;
  case TW_SR_DATA_NACK: // done receive, because slave does not want anymore (sent nack)
    receiveByte();
    // falls through
  case TW_SR_STOP: // done receive, because bus stop sent by master
#ifdef TWI_DEBUG
    Serial.println('.');
#endif
    twiSlaveReceive(_done_size, false);
    TWCR |= _BV(TWEA);
    break;
  // Slave transmit states
  case TW_ST_SLA_ACK:
    start();
    twiSlaveTransmit(0, true); // request intial bytes to transmit
    // falls throuhg
  case TW_ST_DATA_ACK:
    transmitByte();
    if (_rem_size == 0 && _more)
      twiSlaveTransmit(_done_size, true); // request more bytes to transmit
    nackIfDone();
    break;
  case TW_ST_DATA_NACK: // done transmit
  case TW_ST_LAST_DATA:
#ifdef TWI_DEBUG
    Serial.println('.');
#endif
    twiSlaveTransmit(_done_size, false); // done transmit
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
