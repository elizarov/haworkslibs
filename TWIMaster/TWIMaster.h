/*
  TWI Master implementation that is not based on interrupts and supports different speeds.
  It plays nicely with TWI Slave code (restores TWI hardware to the previous state it was initially encountered in).

  This code works for regular Arduino Uno/Duemilanove/Pro boards only based on ATmega328p.

  On these boards:
    SDA --> A4 
    SCL --> A5

  Author: Roman Elizarov
*/

#ifndef TWI_MASTER_H_
#define TWI_MASTER_H_

#ifndef __AVR_ATmega328P__
#error "Only ATmega328P is supported"
#endif

#include <Arduino.h>
#include <Timeout.h>
#include <stdint.h>

// high byte -- prescaler bits, low bytes -- TWBR value
typedef uint16_t twi_speed_t;

/*
   SPEED = F_CPU / (16 + 2 * TWBR * prescaler)
   TWBR = (F_CPU / SPEED - 16) / (2 * prescaler)
 */

const twi_speed_t TWI_10K  = 0x100 | uint8_t((F_CPU / 10000 - 16) / 8);  // 100us clock; use prescaler == 4, for F_CPU=16MHz, TWBR=198
const twi_speed_t TWI_40K  = 0x000 | uint8_t((F_CPU / 40000 - 16) / 2);  //  25us clock; use prescaler == 1, for F_CPU=16MHz, TWBR=192
const twi_speed_t TWI_100K = 0x000 | uint8_t((F_CPU / 100000 - 16) / 2); //  10us clock; use prescaler == 1, for F_CPU=16MHz, TWBR=72

const twi_speed_t TWI_DEFAULT_SPEED = TWI_40K; // compromise speed, 100K is a bit too fast for breadboards

class TWIMasterClass {
public:
  TWIMasterClass(twi_speed_t speed = TWI_DEFAULT_SPEED);

  // setSpeed is optional, use only to change speed if != TWI_DEFAULT_SPEED is needed 
  void setSpeed(twi_speed_t speed = TWI_DEFAULT_SPEED);

  // result != 0 means error, error code is returned
  uint8_t transmit(uint8_t addr, const void *tx, uint8_t n, bool keepBus = false);
  // result != 0 means error, error code is returned
  uint8_t receive(uint8_t addr, void *rx, uint8_t n, bool keepBus = false);
  // stop is optinal, transmit / receive call it when used with keepBus = false
  void stop(); 

  // high-level convenience methods
  template<typename T> inline uint8_t transmit(uint8_t addr, const T& tx) { 
    return transmit(addr, &tx, sizeof(tx)); 
  }

  template<typename R> inline uint8_t receive(uint8_t addr, R& rx) { 
    return receive(addr, &rx, sizeof(rx)); 
  }

  template<typename T, typename R> inline uint8_t transmitReceive(uint8_t addr, const T& tx, R& rx) { 
    uint8_t status = transmit(addr, &tx, sizeof(tx), true);
    if (status != 0)
      return status;
    return receive(addr, &rx, sizeof(rx));
  }

private:
  uint8_t _twcr;
  bool _active;

  uint8_t wait(uint8_t expected);
  uint8_t start();
  void abort();
  void restore();
};

// Singleton instance 
extern TWIMasterClass TWIMaster;

#endif
