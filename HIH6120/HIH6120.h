/*
  Honeywell HIH6120 I2C temperature and humidity sensor driver.
  Connect sensor to I2C pins:
  
  +-------+
  | *     |
  +-------+
   | | | |
   1 2 3 4

  1 - VDD 
  2 - GND
  3 - SCL --> A5
  4 - SDA --> A4

  Author: Roman Elizarov
*/

#ifndef HIH6120_H_
#define HIH6120_H_

#include <Arduino.h>
#include <FixNum.h>
#include <Timeout.h>
#include <TWIMaster.h>

class HIH6120 {
public:
  typedef fixnum16_1 temp_t;  
  typedef fixnum16_1 rh_t;  

  HIH6120();
  bool check();        // true when something change (new reading taken or old one is cleared on too many errors)
  temp_t getTemp();
  rh_t getRH();
  uint16_t getState(); // last error code and status for debugging

private:
  bool _valid;
  temp_t _temp;
  rh_t _rh;
  Timeout _timeout;
  uint8_t _state; // 0 - send measure cmd, 1 - receive measurement
  uint8_t _last_error;
  uint8_t _retry_count;

  uint8_t receive();
  bool retry();
};

// ------------ short method implementations are inline here ------------

inline HIH6120::temp_t HIH6120::getTemp() {
  return _temp;
}

inline HIH6120::rh_t HIH6120::getRH() {
  return _rh;
}

inline uint16_t HIH6120::getState() {
  return (_last_error << 8) | _state;
}

#endif
