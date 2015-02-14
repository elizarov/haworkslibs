#ifndef HIH6120_H_
#define HIH6220_H_

#include <Arduino.h>
#include <FixNum.h>
#include <Timeout.h>
#include <TWIMaster.h>

class HIH6120 {
public:
  typedef fixnum16_1 rh_t;  
  typedef fixnum16_1 temp_t;  

  HIH6120(twi_speed_t twi_speed);
  bool check();
  rh_t getRH();
  temp_t getTemp();
  uint16_t getState(); // last error code and status for debugging

private:
  static const uint8_t BYTES = 4;

  struct Data {
    uint16_t h; // rh
    uint16_t t; // temp 
    
    uint8_t set(uint8_t (&b)[BYTES]);
  };

  twi_speed_t _twi_speed;
  Data _data;
  Timeout _timeout;
  bool _measure;
  bool _valid;
  uint8_t _last_error;
  uint8_t _retry_count;

  uint8_t receive();
  void retry();
};

#endif
