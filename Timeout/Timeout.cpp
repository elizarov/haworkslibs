#include "Timeout.h"

bool Timeout::check() {
  if (!enabled())
    return false;
  // Note: Arduino millis() type is unsigned. Need to convert back to signed type for wrapping check
  if (type(_time - millis()) <= 0) {
    disable();
    return true;
  }
  return false;
}

Timeout::type Timeout::remaining() {
  if (!enabled())
    return 0;
  // Note: Arduino millis() type is unsigned. Need to convert back to signed type for wrapping check
  type rem = type(_time - millis());
  if (rem > 0)
    return rem;
  return 0;
}

void Timeout::reset(Timeout::type interval) {
  if (interval < 0)
    interval = 0; // assume zero interval was requested
  _time = millis() + interval;  
  if (_time == 0) // just in case
    _time = 1;
}
