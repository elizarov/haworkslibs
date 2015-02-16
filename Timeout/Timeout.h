/*
 Simple timeout class that "fires" once by returning true from its "check" method when
 previously spcified time interval passes. Auto-repeat is not supported. Call "reset".

 Basic usage:

   Timeout timeout(initial); // will fire after initial interval

 To check that timeout had passed:

   // Invoke check periodically or it will wrap in ~24 days
   if (timeout.check()) {
     .. // do something
     timeout.reset(internval); // don't foreget to reset to new interval
   }

  Author: Roman Elizarov
*/

#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include <Arduino.h>

class Timeout {
public:
  typedef long type; // Arduino is using "unsigned long" in millis(), but we use signed for wrapping checks

  static const type SECOND = 1000UL;
  static const type MINUTE = 60 * SECOND;
  static const type HOUR = 60 * MINUTE;
  static const type DAY = 24 * HOUR;
  
  Timeout();
  Timeout(type interval);

  bool check();              // returns true (once!) when done, must call periodially when enabled to avoid wrap
  bool enabled();            // returns true when still enabled (fast check)
  type remaining();          // returns how much time remainig of 0 when disabled / done, does not replace calling check periodically

  void disable();            // disables timeout 
  void reset(type interval); // starts / resets to a specified interval

private:
  type _time;
};

// ------------ short method implementations are inline here ------------

inline Timeout::Timeout() {}

inline Timeout::Timeout(Timeout::type interval) { 
  reset(interval);
}

inline bool Timeout::enabled() {
  return _time != 0;
}

inline void Timeout::disable() {
  _time = 0;
}

#endif
