/*
  Honeywell HumidIcon HIHxxx-020 I2C temperature and humidity sensors driver.
  This includes HIH6000, HIH6100, HIH8000, and HIH9000 series.

  Connect sensor in SIP package to I2C pins in the following way:
  
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

#ifndef HIH_H_
#define HIH_H_

#include <Arduino.h>
#include <FixNum.h>
#include <Timeout.h>
#include <TWIMaster.h>

class HIH {
public:
  typedef fixnum16_1 temp_t;  
  typedef fixnum16_1 rh_t;  

  HIH();
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

inline HIH::temp_t HIH::getTemp() {
  return _temp;
}

inline HIH::rh_t HIH::getRH() {
  return _rh;
}

inline uint16_t HIH::getState() {
  return (_last_error << 8) | _state;
}

#endif
