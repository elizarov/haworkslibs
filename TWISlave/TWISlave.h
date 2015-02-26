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

#if !defined(__AVR_ATmega328P__) && !defined(__AVR_ATmega168P__)
#error "Only ATmegaXX8P is supported"
#endif

#include <stdint.h>

class TWISlaveClass {
public:
  void begin(uint8_t addr);
  void use(void *buf, uint8_t size, bool more = false);

  // convenience method
  template<typename T> inline void use(T& buf) { use(&buf, sizeof(buf)); }

  // internal use only
  void _processInterrupt();

private:
  uint8_t *_buf;
  uint8_t _rem_size;
  uint8_t _done_size;
  bool _more;

  void start();
  void nackIfDone();
  void receiveByte();
  void transmitByte();
};

extern TWISlaveClass TWISlave;

void twiSlaveReceive(uint8_t doneSize, bool more);
void twiSlaveTransmit(uint8_t doneSize, bool more);

#endif
