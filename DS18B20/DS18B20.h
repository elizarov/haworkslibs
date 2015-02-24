/*
  Arduino library for DS18B20 sensor. It does oversampling and computes running average,
  resulitng in a very high-precision and stable temperature measurement.

         Wiring:
      +---------+
      | DALLAS  |
      | DS18B20 |
      |         |
      +---------+
        |  |  |
        1  2  3

     1 - GND
     2 - DATA
     3 - VCC

  Author: Roman Elizarov
*/

#ifndef DS18B20_H_
#define DS18B20_H_

#include <Arduino.h>
#include <OneWire.h>
#include <Timeout.h>
#include <FixNum.h>

class DS18B20 {
public:
  typedef fixnum16_2 temp_t;

  DS18B20(uint8_t pin);

  bool check();
  temp_t getTemp(); // Returns value in 1/100 of degree Centigrade (oversampling!)
  uint8_t getLastError(); // last error code and status for debugging

private:
  static const uint8_t QUEUE_SIZE = 12;
  static const int16_t NO_VAL = INT_MAX;
  
  OneWire _wire;
  Timeout _timeout;
  uint8_t _index;
  uint8_t _size;
  int16_t _queue[QUEUE_SIZE]; // queue of raw reads in 1/16 of degree Centigrade
  uint8_t _fail_count;
  uint8_t _last_error;
  
  temp_t _value; // computed value

  void clear();
  bool fail();
  int16_t readScratchPad();
  bool startConversion();
  void computeValue();
};

#endif /* DS18B20_H_ */
