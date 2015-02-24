/*
  Modified / improved DS3231 library (originally by Seeed). 
  The general improvement theme is reliability of the library, so that it works despite transient errors.
  A list of modifications:
  - Use reliable TWIMaster libary (that does not hang in multi-master setups)
  - Key methods now indicate whether they were successful 
  - Removed day of week support from DateTime class (not really needed)
  - Handle the case of uknown / uninitialized date time
  - Removed 2000 offset to years (keep it down to hardware at two digits)
  - Added formatting in ISO "YY-MM-DD HH:MM:SS" format
  - Support for oversampled temperature reading via a separate class and without floating point (uses FixNum)

  Author: Roman Elizarov
*/

// DS3231 Class is by Seeed Technology Inc(http://www.seeedstudio.com) and used
// in Seeeduino Stalker v2.1 for battery management(MCU power saving mode)
// & to generate timestamp for data logging. DateTime Class is a modified
// version supporting day-of-week.

// Original DateTime Class and its utility code is by Jean-Claude Wippler at JeeLabs
// http://jeelabs.net/projects/cafe/wiki/RTClib 
// Released under MIT License http://opensource.org/licenses/mit-license.php

#ifndef DS3231_H_
#define DS3231_H_

#include <stdint.h>

// uses the following haworkslibs
#include <TWIMaster.h>
#include <Timeout.h>
#include <FixNum.h>

// periodicity constants for enableInterrupts(uint8_t periodicity)
const uint8_t EverySecond = 0x01;
const uint8_t EveryMinute = 0x02;
const uint8_t EveryHour   = 0x03;

// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime {
public:
  inline DateTime() {}; // all zeros, 00-00-00 00:00:00, uninitialized time
  DateTime(long t); // 32-bit time as seconds since 00-01-01 (Jan 1st, 2000)
  DateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec);
  DateTime(const char* date, const char* time);

  inline uint8_t second() const      { return ss; }
  inline uint8_t minute() const      { return mm; } 
  inline uint8_t hour() const        { return hh; }

  inline uint8_t date() const        { return d; }
  inline uint8_t month() const       { return m; }
  inline uint8_t year() const        { return y; }

  // 32-bit time as seconds since 1/1/2000
  long get() const;   

  // true when time is define (non-zero)
  explicit operator bool() { return y != 0 || m != 0 || d != 0 || hh != 0 || mm != 0 || ss != 0; }

  // char array type to keep string representation as YY-MM-DD HH:MM:SS
  typedef char str_t[18];

  // Buffer class with temporary char array
  class Buf {
  private:
    str_t _buf;
  public:
    Buf(const DateTime& dt);
    inline operator char*() { return _buf; }
  };

  // formatting
  inline Buf format() { return Buf(*this); }

private:
    uint8_t y, m, d, hh, mm, ss;
};

// RTC DS3231 chip connected via I2C and uses the TWIMaster library.
// Only 24 Hour time format is supported in this implementation
class DS3231 {
public:
  bool adjust(const DateTime& dt);  //Changes the date-time
  DateTime now();                   //Gets the current date-time

  //Decides the /INT pin's output setting
  //periodicity can be any of following defines: EverySecond, EveryMinute, EveryHour 
  bool enableInterrupts(uint8_t periodicity);
  bool enableInterrupts(uint8_t hh24, uint8_t mm, uint8_t ss);
  bool disableInterrupts();
  bool clearINTStatus();

  inline uint8_t getLastError() { return _last_error; } // for debugging I2C bus problems, see TWIMaster library

protected:
  uint8_t readRegister(uint8_t regaddress);
  bool writeRegister(uint8_t regaddress, uint8_t value);

  uint8_t _last_error;
};

// ========================================================================
// DS3231Temp class for oversampled temperature

class DS3231Temp : public DS3231 {
public:
  typedef fixnum16_2 temp_t;

  DS3231Temp();

  bool check(); // call periodically to force temperature readings
  temp_t getTemp(); // get current temperature reading

private:
  static const uint8_t QUEUE_SIZE = 12;
  static const int16_t NO_VAL = 0x7fff;
  
  Timeout _timeout;
  uint8_t _index;
  uint8_t _size;
  int16_t _queue[QUEUE_SIZE]; // queue of raw reads in 1/4 of degree Centigrade
  uint8_t _fail_count;
  temp_t _temp;

  void clear();
  bool startConversion();
  bool fail();
  void computeTemp();
  int16_t readTemp();
};

#endif
