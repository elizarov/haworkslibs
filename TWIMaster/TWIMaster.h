/*
   TWI Master implementation that is not based on interruptes, supporits different speeds,
   and restores TWI hardware to the previous state it was initially encountered in.
*/

#ifndef TWI_MASTER_H_
#define TWI_MASTER_H_

#include <Arduino.h>
#include <Timeout.h>
#include <stdint.h>

// high byte -- prescaler bits, low bytes -- TWBR value
typedef uint16_t twi_speed_t;

/*
   SPEED = F_CPU / (16 + 2 * TWBR * prescaler)
   TWBR = (F_CPU / SPEED - 16) / (2 * prescaler)
 */

const twi_speed_t TWI_10K  = 0x100 | uint8_t((F_CPU / 10000 - 16) / 8); // use prescaler == 4, for F_CPU=16MHz, TWBR=198
const twi_speed_t TWI_100K = 0x000 | uint8_t((F_CPU / 100000 - 16) / 2); // use prescaler == 1, for F_CPU=16MHz, TWBR=72

class TWIMaster {
public:
  TWIMaster(twi_speed_t speed);
  ~TWIMaster();
  void stop(); // optional, destructor calls it
  // result != 0 means error, error code is returned
  uint8_t transmit(uint8_t addr, uint8_t* buf, uint8_t n);
  // result != 0 means error, error code is returned
  uint8_t receive(uint8_t addr, uint8_t* buf, uint8_t n);

private:
  uint8_t _twbr;
  uint8_t _twcr;
  uint8_t _twsr;
  uint8_t _active;

  // result != 0 means error, error code is returned
  uint8_t start();

};

#endif
