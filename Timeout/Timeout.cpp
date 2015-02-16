#include "Timeout.h"

bool Timeout::check() {
  if (!enabled())
    return false;
  if (signedType(_time - millis()) <= 0) {
    disable();
    return true;
  }
  return false;
}

Timeout::type Timeout::remaining() {
  if (!enabled())
    return 0;
  long rem = signedType(_time - millis());
  if (rem > 0)
    return rem;
  return 0;
}

void Timeout::reset(Timeout::type interval) {
  _time = millis() + interval;  
  if (_time == 0) // just in case
    _time = 1;
}
