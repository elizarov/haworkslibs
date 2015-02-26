/*
  TWI Slave implementation without function pointers and with reliable state machine.

  This code works for regular Arduino Uno/Duemilanove/Pro boards only based on ATmega328p.

  On these boards:
    SDA --> A4 
    SCL --> A5

  Author: Roman Elizarov
*/


#ifndef TWI_SLAVE_H_
#define TWI_SLAVE_H_

#ifndef __AVR_ATmega328P__
#error "Only ATmega328P is supported"
#endif

#include <Arduino.h>
#include <stdint.h>

class TWISlaveClass {
public:
  void begin(uint8_t addr);
  void use(void *buf, uint8_t size);

  // convenience method
  template<typename T> inline void use(T& buf) { use(&buf, sizeof(buf)); }

  // internal use only
  void _processInterrupt();

private:
  uint8_t *_buf;
  uint8_t _rem_size;
  uint8_t _done_size;

  void nackIfDone();
  void receiveByte();
  void transmitByte();
};

extern TWISlaveClass TWISlave;

void twiSlaveCall(uint8_t addr, bool slaveTransmit);
void twiSlaveDone(uint8_t size, bool slaveTransmit);

#endif
